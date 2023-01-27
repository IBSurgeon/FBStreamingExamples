#include "SimpleJsonPlugin.h"

#include <sstream>
#include <fstream>
#include "../../common/FBAutoPtr.h"
#include "../../common/Utils.h"
#include "../../common/FBCharsetInfo.h"
#include "../../common/LazyFactory.h"

using namespace Firebird;
using namespace std;

struct vary
{
	USHORT vary_length;
	char   vary_string[1]; /* CVC: The original declaration used UCHAR. */
};

namespace SimpleJsonPlugin
{

	static const char* states[] =
	{
		"free",
		"used",
		"full",
		"archive"
	};

	class SimpleJsonPlugin::PluginImp
	{
	private:
		json doc;
	public:
		PluginImp();
		void writeHeader(const SegmentHeaderInfo& headerInfo);
		void writeEvent(const json& event);
		void saveToFile(const fs::path& fileName);
	};

	SimpleJsonPlugin::PluginImp::PluginImp()
		: doc()
	{
	}

	void SimpleJsonPlugin::PluginImp::writeHeader(const SegmentHeaderInfo& headerInfo)
	{
		// reset
		doc = {};
		
		doc["header"] = {};
		doc["events"] = json::array();

		json header;
		header["version"] = headerInfo.version;
		header["guid"] = headerInfo.guid;
		header["sequence"] = headerInfo.sequence;
		header["state"] = states[headerInfo.state];

		doc["header"] = header;
	}

	void SimpleJsonPlugin::PluginImp::writeEvent(const json& event)
	{
		doc["events"].push_back(event);
	}

	void SimpleJsonPlugin::PluginImp::saveToFile(const fs::path& fileName)
	{
		std::ofstream o(fileName);
		o << std::setw(4) << doc << std::endl;
		o.close();

		// reset
		doc = {};
	}

	SimpleJsonPlugin::SimpleJsonPlugin(IMaster* master, IConfig* config, IStreamLogger* logger)
		: m_master(master)
		, m_config(config)
		, m_logger(logger)
		, m_owner(nullptr)
		, m_refCounter(0)
		, m_transactions()
		, m_att(nullptr)
		, m_util(master->getUtilInterface())
		, m_encoders()
		, m_segmentHeader()
		, m_include_tables(nullptr)
		, m_exclude_tables(nullptr)
		, m_dumpBlobs(false)
		, m_registerDDL(true)
		, m_registerSequence(true)
		, m_outputPath()
		, pImp(make_unique<PluginImp>())
	{
		m_config->addRef();
		m_logger->addRef();

		Firebird::CheckStatusWrapper status(master->getStatus());

		AutoRelease<IConfigEntry> ceDumpBlobs(m_config->find(&status, "dumpBlobs"));
		if (ceDumpBlobs) {
			m_dumpBlobs = ceDumpBlobs->getBoolValue();
		}

		AutoRelease<IConfigEntry> ceDDLEvents(m_config->find(&status, "register_ddl_events"));
		if (ceDDLEvents) {
			m_registerDDL = ceDDLEvents->getBoolValue();
		}

		AutoRelease<IConfigEntry> ceSequenceEvents(m_config->find(&status, "register_sequence_events"));
		if (ceSequenceEvents) {
			m_registerSequence = ceSequenceEvents->getBoolValue();
		}

		AutoRelease<IConfigEntry> ceOutputDir(m_config->find(&status, "outputDir"));
		if (ceOutputDir) {
			m_outputPath.assign(ceOutputDir->getValue());
		}

	}

	SimpleJsonPlugin::~SimpleJsonPlugin()
	{
		if (m_att)
			m_att->release();
		if (m_logger)
			m_logger->release();
		if (m_config)
			m_config->release();
	}

	void SimpleJsonPlugin::addRef()
	{
		++m_refCounter;
	}

	int SimpleJsonPlugin::release()
	{
		if (--m_refCounter == 0)
		{
			delete this;
			return 0;
		}
		return 1;
	}


	void SimpleJsonPlugin::setOwner(IReferenceCounted* o)
	{
		m_owner = o;
	}

	IReferenceCounted* SimpleJsonPlugin::getOwner()
	{
		return m_owner;
	}

	FB_BOOLEAN SimpleJsonPlugin::init(ThrowStatusWrapper* status, IAttachment* attachment)
	{
		if (m_att) {
			m_att->release();
			m_att = nullptr;
		}
		m_att = attachment;
		if (m_att)
			m_att->addRef();


		AutoRelease<IConfigEntry> ceIncludeTables(m_config->find(status, "include_tables"));
		if (ceIncludeTables) {
			try {
				m_include_tables = make_unique<std::regex>(ceIncludeTables->getValue());
			}
			catch (const std::regex_error& e) {
				throwException(status, e.what());
			}
		}

		AutoRelease<IConfigEntry> ceExludeTables(m_config->find(status, "exclude_tables"));
		if (ceExludeTables) {
			try {
				m_exclude_tables = make_unique<std::regex>(ceExludeTables->getValue());
			}
			catch (const std::regex_error& e) {
				throwException(status, e.what());
			}
		}

		return FB_TRUE;
	}

	void SimpleJsonPlugin::finish(ThrowStatusWrapper* status)
	{
		m_include_tables = nullptr;
		m_exclude_tables = nullptr;
	}

	void SimpleJsonPlugin::startSegment(ThrowStatusWrapper* status, SegmentHeaderInfo* segmentHeader)
	{
		// copy to internal segmentHeader
		m_segmentHeader.version = segmentHeader->version;
		m_segmentHeader.sequence = segmentHeader->sequence;
		m_segmentHeader.state = segmentHeader->state;
		m_segmentHeader.length = segmentHeader->length;
		memcpy(m_segmentHeader.name, segmentHeader->name, std::size(segmentHeader->name));
		memcpy(m_segmentHeader.guid, segmentHeader->guid, std::size(segmentHeader->guid));

		if (m_logger->getLevel() <= IStreamLogger::LEVEL_DEBUG) {
			// if the debug level is set, print the segment header
			stringstream ss;

			ss << std::endl;
			ss << setw(80) << std::setfill('=') << "=" << std::endl;
			ss << "Segment: " << m_segmentHeader.name << std::endl;
			ss << "Version: " << m_segmentHeader.version << std::endl;
			ss << "Guid: " << m_segmentHeader.guid << std::endl;
			ss << "Sequence: " << m_segmentHeader.sequence << std::endl;
			ss << "State: " << states[m_segmentHeader.state] << std::endl;
			ss << "Length: " << m_segmentHeader.length << std::endl;
			ss << setw(80) << std::setfill('=') << "=" << std::endl;
			ss << std::endl;

			m_logger->debug(ss.str().c_str());
		}

		pImp->writeHeader(m_segmentHeader);
	}

	void SimpleJsonPlugin::finishSegment(ThrowStatusWrapper* status)
	{
		string segmentName = m_segmentHeader.name;
		fs::path fileName = m_outputPath / (segmentName + ".json");

		pImp->saveToFile(fileName);
	}

	IApplierTransaction* SimpleJsonPlugin::startTransaction(ThrowStatusWrapper* status, ISC_INT64 number)
	{
		m_logger->debug(vformat("[%" UQUADFORMAT "] START", number).c_str());
		auto tra = new ReplTransaction(this, number);
		m_transactions[number] = tra;
		return tra;
	}

	void SimpleJsonPlugin::setSequence(ThrowStatusWrapper* status, const char* name, ISC_INT64 value)
	{
		if (!m_registerSequence) {
			// If registration of the sequence value setting event is disabled, then exit.
			return;
		}

		json jEvent;

		jEvent["event"] = "SET SEQUENCE";
		jEvent["sequence"] = name;
		jEvent["value"] = static_cast<int64_t>(value);

		if (m_logger->getLevel() <= IStreamLogger::LEVEL_DEBUG) {
			std::stringstream ss;
			ss << std::endl;
			ss << std::setw(4) << jEvent << std::endl;
			const std::string message = ss.str();
			m_logger->debug(message.c_str());
		}

		pImp->writeEvent(jEvent);
	}

	FB_BOOLEAN SimpleJsonPlugin::matchTable(ThrowStatusWrapper* status, const char* relationName)
	{
		bool match = true;
		if (m_include_tables != nullptr) {
			// The table name must match the regular expression
			match = match && std::regex_match(relationName, *m_include_tables);
		}
		if (m_exclude_tables != nullptr) {
			// Table name must not match regular expression
			match = match && !std::regex_match(relationName, *m_exclude_tables);
		}
		return match;
	}

	IApplierTransaction* SimpleJsonPlugin::getTransaction(ThrowStatusWrapper* status, ISC_INT64 number)
	{
		const auto it = m_transactions.find(number);
		if (it == m_transactions.end()) {
			throwException(status, "Transaction %" UQUADFORMAT " not found, segment name %s", number, m_segmentHeader.name);
		}
		auto tra = (*it).second;
		return tra;
	}

	void SimpleJsonPlugin::cleanupTransaction(ThrowStatusWrapper* status, ISC_INT64 number)
	{
		m_transactions.erase(number);
	}

	void SimpleJsonPlugin::cleanupTransactions(ThrowStatusWrapper* status)
	{
		// get a list of transaction numbers
		vector<ISC_INT64> numbers;
		transform(std::begin(m_transactions), std::end(m_transactions), back_inserter(numbers),
			[](auto const& pair) {
				return pair.first;
			});
		// rollback all transactions
		for (auto traNum : numbers)
		{
			auto tra = m_transactions[traNum];
			tra->rollback(status);
			tra->dispose();
			m_transactions.erase(traNum);
		}
	}

	void SimpleJsonPlugin::log(unsigned level, const char* message)
	{
		m_logger->log(level, message);
	}

	const FBStringEncoderPtr& SimpleJsonPlugin::getEncoder(unsigned charsetId)
	{
		auto [it, result] = m_encoders.try_emplace(charsetId, lazy_convert_construct([charsetId] {return std::make_unique<FBStringEncoder>(charsetId);  }));
		return it->second;
	}

	ReplTransaction::ReplTransaction(SimpleJsonPlugin* applier, ISC_INT64 number)
		: m_applier(applier)
		, m_number(number)
		, m_savepoints()
		, m_blobs()
	{
		m_applier->addRef(); // Lock parent from disappearing
	}

	ReplTransaction::~ReplTransaction()
	{
		m_applier->release();
	}

	void ReplTransaction::dispose()
	{
		delete this;
	}

	void ReplTransaction::prepare(ThrowStatusWrapper* status)
	{
		// ignore
	}

	void ReplTransaction::commit(ThrowStatusWrapper* status)
	{
		m_applier->m_logger->debug(vformat("[%" UQUADFORMAT "] COMMIT", m_number).c_str());
		if (m_savepoints.size() > 0) {
			auto events = std::move(m_savepoints.top());
			if (events->size() > 0) {
				if (m_applier->m_logger->getLevel() <= IStreamLogger::LEVEL_DEBUG) {
					const std::string message = "Events: "s + std::to_string(events->size());
					m_applier->m_logger->debug(message.c_str());
				}
				// add events
				try {
					for (const auto& ev : *events) {
						m_applier->pImp->writeEvent(ev);
					}
				}
				catch (const std::exception& e) {
					throwException(status, e.what());
				}

			}
			// clear remaining savepoints
			while (m_savepoints.size() > 0) {
				m_savepoints.pop();
			}
		}
	}

	void ReplTransaction::rollback(ThrowStatusWrapper* status)
	{
		while (m_savepoints.size() > 0) {
			m_savepoints.pop();
		}
	}

	void ReplTransaction::startSavepoint(ThrowStatusWrapper* status)
	{
		m_savepoints.push(make_unique<EventList>());
	}

	void ReplTransaction::releaseSavepoint(ThrowStatusWrapper* status)
	{
		if (m_savepoints.size() > 1) {
			auto prevEvents = std::move(m_savepoints.top());
			m_savepoints.pop();
			const auto& events = m_savepoints.top();
			events->insert(events->end(), prevEvents->begin(), prevEvents->end());
		}
	}

	void ReplTransaction::rollbackSavepoint(ThrowStatusWrapper* status)
	{
		m_savepoints.pop();
	}

	void ReplTransaction::insertRecord(ThrowStatusWrapper* status, const char* name, IReplicatedRecord* record)
	{
		if (record->getCount() == 0) {
			m_applier->m_logger->info(vformat("INSERT %s", name).c_str());
			std::string msg = vformat(R"(Format not found. Segment name %s)", m_applier->m_segmentHeader.name);
			m_applier->m_logger->warning(msg.c_str());
			return;
		}
		m_applier->m_logger->debug(vformat("[%" UQUADFORMAT "] INSERT %s (length: %d)", m_number, name, record->getRawLength()).c_str());


		try {
			json jRecord;
			json jBlobs;

			dumpRecord(status, record, jRecord, m_applier->m_dumpBlobs, &jBlobs);

			json jEvent;

			jEvent["event"] = "INSERT";
			jEvent["table"] = name;
			jEvent["tnx"] = static_cast<int64_t>(m_number);
			jEvent["record"] = jRecord;


			if (m_applier->m_dumpBlobs) {
				if (!jBlobs.empty()) {
					jEvent["newBlobs"] = jBlobs;
				}
			}


			if (m_applier->m_logger->getLevel() <= IStreamLogger::LEVEL_DEBUG) {
				std::stringstream ss;
				ss << std::endl;
				ss << std::setw(4) << jEvent << std::endl;
				const std::string message = ss.str();
				m_applier->m_logger->debug(message.c_str());
			}

			// Save event to savepoint
			m_savepoints.top()->push_back(jEvent);
		}
		catch (const std::exception& e) {
			throwException(status, e.what());
		}
	}

	void ReplTransaction::updateRecord(ThrowStatusWrapper* status, const char* name, IReplicatedRecord* orgRecord, IReplicatedRecord* newRecord)
	{
		if (orgRecord->getCount() == 0) {
			m_applier->m_logger->info(vformat("UPDATE %s", name).c_str());
			std::string msg = vformat(R"(No format found for old record table. Segment name %s)", m_applier->m_segmentHeader.name);
			m_applier->m_logger->warning(msg.c_str());
			return;
		}
		if (newRecord->getCount() == 0) {
			m_applier->m_logger->info(vformat("UPDATE %s", name).c_str());
			std::string msg = vformat(R"(No format found for new record table. Segment name %s)", m_applier->m_segmentHeader.name);
			m_applier->m_logger->warning(msg.c_str());
			return;
		}
		m_applier->m_logger->debug(vformat("[%" UQUADFORMAT "] UPDATE %s (orgLength: %d, newLength: %d)", m_number, name, orgRecord->getRawLength(), newRecord->getRawLength()).c_str());

		try {
			json jOrgRecord;
			json jNewRecord;
			json jBlobs;

			dumpRecord(status, orgRecord, jOrgRecord, false, nullptr);
			dumpRecord(status, newRecord, jNewRecord, m_applier->m_dumpBlobs, &jBlobs);

			set<std::string> changedFields;
			for (const auto& [key, value] : jNewRecord.items()) {
				const auto iOldElem = jOrgRecord.find(key);
				if (iOldElem == jOrgRecord.cend()) {
					changedFields.insert(std::string(key));
				}
				else {
					const auto& oldVal = iOldElem.value();
					if (value != oldVal) {
						changedFields.insert(std::string(key));
					}
				}
			}

			json jEvent;

			jEvent["event"] = "UPDATE";
			jEvent["table"] = name;
			jEvent["tnx"] = static_cast<int64_t>(m_number);
			jEvent["oldRecord"] = changedFields;
			jEvent["oldRecord"] = jOrgRecord;
			jEvent["record"] = jNewRecord;


			if (m_applier->m_dumpBlobs) {
				if (!jBlobs.empty()) {
					jEvent["newBlobs"] = jBlobs;
				}
			}

			if (m_applier->m_logger->getLevel() <= IStreamLogger::LEVEL_DEBUG) {
				std::stringstream ss;
				ss << std::endl;
				ss << std::setw(4) << jEvent << std::endl;
				const std::string message = ss.str();
				m_applier->m_logger->debug(message.c_str());
			}

			// Save event to savepoint
			m_savepoints.top()->push_back(jEvent);
		}
		catch (const std::exception& e) {
			throwException(status, e.what());
		}
	}

	void ReplTransaction::deleteRecord(ThrowStatusWrapper* status, const char* name, IReplicatedRecord* record)
	{
		if (record->getCount() == 0) {
			m_applier->m_logger->info(vformat("DELETE %s", name).c_str());
			std::string msg = vformat(R"(Format not found. Segment name %s)", m_applier->m_segmentHeader.name);
			m_applier->m_logger->warning(msg.c_str());
			return;
		}
		m_applier->m_logger->debug(vformat("[%" UQUADFORMAT "] DELETE %s (length: %d)", m_number, name, record->getRawLength()).c_str());

		try {
			json jRecord;
			dumpRecord(status, record, jRecord, false, nullptr);

			json jEvent;

			jEvent["event"] = "DELETE";
			jEvent["table"] = name;
			jEvent["tnx"] = static_cast<int64_t>(m_number);
			jEvent["record"] = jRecord;

			if (m_applier->m_logger->getLevel() <= IStreamLogger::LEVEL_DEBUG) {
				std::stringstream ss;
				ss << std::endl;
				ss << std::setw(4) << jEvent << std::endl;
				const std::string message = ss.str();
				m_applier->m_logger->debug(message.c_str());
			}

			// Save event to savepoint
			m_savepoints.top()->push_back(jEvent);
		}
		catch (const std::exception& e) {
			throwException(status, e.what());
		}
	}

	void ReplTransaction::executeSql(ThrowStatusWrapper* status, const char* sql)
	{
		if (!m_applier->m_registerDDL) {
			// If registration of DDL events is disabled, exit.
			return;
		}

		json jEvent;

		jEvent["event"] = "EXECUTE SQL";
		jEvent["sql"] = sql;
		jEvent["tnx"] = static_cast<int64_t>(m_number);

		if (m_applier->m_logger->getLevel() <= IStreamLogger::LEVEL_DEBUG) {
			std::stringstream ss;
			ss << std::endl;
			ss << std::setw(4) << jEvent << std::endl;
			const std::string message = ss.str();
			m_applier->m_logger->debug(message.c_str());
		}

		// Save event to savepoint
		m_savepoints.top()->push_back(jEvent);
	}

	void ReplTransaction::executeSqlIntl(ThrowStatusWrapper* status, unsigned charset, const char* sql)
	{
		if (!m_applier->m_registerDDL) {
			// If registration of DDL events is disabled, exit.
			return;
		}

		const auto& encoder = m_applier->getEncoder(charset);
		const auto utf8Sql = rtrim(encoder->toUtf8(sql));
		executeSql(status, utf8Sql.c_str());
	}

	void ReplTransaction::storeBlob(ThrowStatusWrapper* status, ISC_QUAD* blob_id,
		ISC_INT64 length, const unsigned char* data)
	{
		if (!m_applier->m_dumpBlobs) {
			// If the BLOB dump is disabled, then exit. This will save memory consumption.
			return;
		}

		auto blobId = *reinterpret_cast<ISC_INT64*>(blob_id);
		if ((length > 0) && (data != nullptr)) {
			auto bData = reinterpret_cast<const std::byte*>(data);
			vector<std::byte> blobData;
			blobData.insert(blobData.end(), bData, bData + length);
			m_blobs[blobId] = blobData;
		}
	}

	void ReplTransaction::dumpRecord(ThrowStatusWrapper* status, IReplicatedRecord* record, json& jRecord, bool dumpBlob,
		json* const jBlobs)
	{
		for (unsigned i = 0; i < record->getCount(); i++) {
			auto field = record->getField(i);
			auto fieldType = field->getType();
			auto fieldSubType = field->getSubType();
			auto fieldCharsetId = field->getCharSet();
			std::string fieldName(field->getName());
			fieldName = rtrim(fieldName);
			auto fieldScale = field->getScale();
			auto fieldLength = field->getLength();
			auto fieldData = field->getData();
			if (fieldData == nullptr) {
				jRecord[fieldName] = nullptr;
			}
			else {
				switch (fieldType) {
				case SQL_TEXT:
				{
					if (fieldCharsetId == CS_BINARY) {
						const auto val = getBinaryString(reinterpret_cast<const std::byte*>(fieldData), fieldLength);
						jRecord[fieldName] = val;
					}
					else {
						const auto text = reinterpret_cast<const char*>(fieldData);
						const std::string s(text, fieldLength);
						if ((fieldCharsetId == CS_UTF8) || (fieldCharsetId == CS_NONE)) {
							jRecord[fieldName] = rtrim(s);
						}
						else {
							// character conversion required
							const auto& encoder = m_applier->getEncoder(fieldCharsetId);
							const auto utf8Str = rtrim(encoder->toUtf8(s));
							jRecord[fieldName] = utf8Str;
						}
					}
					break;
				}
				case SQL_VARYING:
				{
					
					const auto varchar = reinterpret_cast<const vary*>(fieldData);
					if (fieldCharsetId == CS_BINARY) {
						const auto val = getBinaryString(reinterpret_cast<const std::byte*>(fieldData) + 2, varchar->vary_length);
						jRecord[fieldName] = val;
					}
					else {
						const std::string s(varchar->vary_string, varchar->vary_length);
						if ((fieldCharsetId == CS_UTF8) || (fieldCharsetId == CS_NONE)) {
							jRecord[fieldName] = s;
						}
						else {
							// character conversion required
							const auto& encoder = m_applier->getEncoder(fieldCharsetId);
							const auto val = encoder->toUtf8(s);
							jRecord[fieldName] = val;
						}
					}
					break;
				}
				case SQL_SHORT:
				{
					const auto value = *reinterpret_cast<const ISC_SHORT*>(fieldData);
					if (fieldScale == 0) {
						jRecord[fieldName] = value;
					}
					else {
						auto val = getScaledInteger(value, fieldScale);
						jRecord[fieldName] = val;
					}
					break;
				}
				case SQL_LONG:
				{
					const auto value = *reinterpret_cast<const ISC_LONG*>(fieldData);
					if (fieldScale == 0) {
						jRecord[fieldName] = static_cast<int32_t>(value);
					}
					else {
						auto val = getScaledInteger(value, fieldScale);
						jRecord[fieldName] = val;
					}
					break;
				}
				case SQL_INT64:
				{
					const auto value = *reinterpret_cast<const ISC_INT64*>(fieldData);
					if (fieldScale == 0) {
						jRecord[fieldName] = static_cast<int64_t>(value);
					}
					else {
						auto val = getScaledInteger(value, fieldScale);
						jRecord[fieldName] = val;
					}
					break;
				}
				case SQL_INT128:
				{
					auto value = reinterpret_cast<const FB_I128*>(fieldData);
					const auto iInt128 = m_applier->m_util->getInt128(status);
					char buffer[IInt128::STRING_SIZE];
					iInt128->toString(status, value, static_cast<int>(fieldScale), IInt128::STRING_SIZE, buffer);
					std::string s(buffer);
					const auto val = rtrim(s);
					jRecord[fieldName] = val;
					break;
				}
				case SQL_FLOAT:
				{
					const auto value = *reinterpret_cast<const float*>(fieldData);
					jRecord[fieldName] = value;
					break;
				}
				case SQL_DOUBLE:
					[[fallthrough]];
				case SQL_D_FLOAT:
				{
					const auto value = *reinterpret_cast<const double*>(fieldData);
					jRecord[fieldName] = value;
					break;
				}
				case SQL_TIMESTAMP:
				{
					const auto value = *reinterpret_cast<const ISC_TIMESTAMP*>(fieldData);
					unsigned year, month, day;
					unsigned hours, minutes, seconds, fractions;
					m_applier->m_util->decodeDate(value.timestamp_date, &year, &month, &day);
					m_applier->m_util->decodeTime(value.timestamp_time, &hours, &minutes, &seconds, &fractions);
					const auto val = vformat("%04d-%02d-%02d %02d:%02d:%02d.%d", year, month, day, hours, minutes, seconds, fractions);
					jRecord[fieldName] = val;
					break;
				}
				case SQL_TYPE_DATE:
				{
					const auto value = *reinterpret_cast<const ISC_DATE*>(fieldData);
					unsigned year, month, day;
					m_applier->m_util->decodeDate(value, &year, &month, &day);
					const auto val = vformat("%04d-%02d-%02d", year, month, day);
					jRecord[fieldName] = val;
					break;
				}
				case SQL_TYPE_TIME:
				{
					const auto value = *reinterpret_cast<const ISC_TIME*>(fieldData);
					unsigned hours, minutes, seconds, fractions;
					m_applier->m_util->decodeTime(value, &hours, &minutes, &seconds, &fractions);
					const auto val = vformat("%02d:%02d:%02d.%d", hours, minutes, seconds, fractions);
					jRecord[fieldName] = val;
					break;
				}
				case SQL_TIMESTAMP_TZ:
				{
					const auto value = reinterpret_cast<const ISC_TIMESTAMP_TZ*>(fieldData);
					unsigned year, month, day;
					unsigned hours, minutes, seconds, fractions;
					char timezoneBuffer[252];
					m_applier->m_util->decodeTimeStampTz(status, value, &year, &month, &day, &hours, &minutes, &seconds, &fractions, 252, timezoneBuffer);
					const auto val = vformat("%04d-%02d-%02d %02d:%02d:%02d.%d %s", year, month, day, hours, minutes, seconds, fractions);
					jRecord[fieldName] = val;
					break;
				}
				case SQL_TIME_TZ:
				{
					const auto value = reinterpret_cast<const ISC_TIME_TZ*>(fieldData);
					unsigned hours, minutes, seconds, fractions;
					char timezoneBuffer[252];
					m_applier->m_util->decodeTimeTz(status, value, &hours, &minutes, &seconds, &fractions, 252, timezoneBuffer);
					const auto val = vformat("%02d:%02d:%02d.%d %s", hours, minutes, seconds, fractions, timezoneBuffer);
					jRecord[fieldName] = val;
					break;
				}
				case SQL_BOOLEAN:
				{
					const auto value = *reinterpret_cast<const FB_BOOLEAN*>(fieldData);
					const auto val = (value ? true : false);
					jRecord[fieldName] = val;
					break;
				}
				case SQL_DEC16:
				{
					auto value = reinterpret_cast<const FB_DEC16*>(fieldData);
					const auto iDecFloat16 = m_applier->m_util->getDecFloat16(status);
					char buffer[IDecFloat16::STRING_SIZE];
					iDecFloat16->toString(status, value, IDecFloat16::STRING_SIZE, buffer);
					std::string s(buffer);
					const auto val = rtrim(s);
					jRecord[fieldName] = val;
					break;
				}
				case SQL_DEC34:
				{
					auto value = reinterpret_cast<const FB_DEC34*>(fieldData);
					const auto iDecFloat34 = m_applier->m_util->getDecFloat34(status);
					char buffer[IDecFloat34::STRING_SIZE];
					iDecFloat34->toString(status, value, IDecFloat34::STRING_SIZE, buffer);
					std::string s(buffer);
					const auto val = rtrim(s);
					jRecord[fieldName] = val;
					break;
				}
				case SQL_BLOB:
				{
					const auto blobId = reinterpret_cast<const ISC_QUAD*>(fieldData);
					const auto val = vformat("%d:%d", blobId->gds_quad_high, blobId->gds_quad_low);
					jRecord[fieldName] = val;
					if (dumpBlob && (jBlobs != nullptr)) {
						auto blobKey = *reinterpret_cast<ISC_INT64 const*>(blobId);
						auto it = m_blobs.find(blobKey);
						if (it != m_blobs.end()) {
							auto blobData = (*it).second;
							std::string val = "";
							if (fieldSubType == 1) {
								std::string text(reinterpret_cast<char*>(blobData.data()), blobData.size());
								if ((fieldCharsetId == CS_UTF8) || (fieldCharsetId == CS_NONE)) {
									val = text;
								}
								else {
									// character conversion required
									const auto& encoder = m_applier->getEncoder(fieldCharsetId);
									val = encoder->toUtf8(text);
								}
							}
							else {
								val = getBinaryString((blobData.data()), blobData.size());
							}
							(*jBlobs)[fieldName] = val;
						}
					}
					break;
				}
				case SQL_ARRAY:
				{
					throwException(status, "Array is not supported");
					break;
				}
				default:
					throwException(status, "Unknown datatype");
				}
			}
		}
	}

	SimpleJsonPluginFactory::SimpleJsonPluginFactory(IMaster* master)
		: m_master(master)
	{}

	IReplicateApplierPlugin* SimpleJsonPluginFactory::createPlugin(ThrowStatusWrapper* status, IConfig* config, IStreamLogger* logger)
	{
		auto plugin = new SimpleJsonPlugin(m_master, config, logger);
		plugin->addRef();
		return plugin;
	}
}
