#include "SimpleJsonPlugin.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <vector>

#include <nlohmann/json.hpp>

#include "../../common/FBAutoPtr.h"
#include "../../common/LazyFactory.h"
#include "../../common/Utils.h"
#include "../../common/charsets.h"
#include "../../encoding/StringConverterHelper.h"
#include "../../encoding/StringEncodeHelper.h"

using namespace Firebird;

namespace SimpleJsonPlugin {

namespace fs = std::filesystem;

using nlohmann::ordered_json;
using FbUtils::IscRandomStatus;

class SimpleJsonStreamPlugin final : public IStreamPluginImpl<SimpleJsonStreamPlugin, ThrowStatusWrapper> {
public:
    SimpleJsonStreamPlugin() = delete;
    SimpleJsonStreamPlugin(IMaster* master, IConfig* config, IStringEncodeUtils* encodeUtils, IStreamLogger* logger);
    virtual ~SimpleJsonStreamPlugin();

    // IReferenceCounted implementation
    void addRef() override;
    int release() override;

    // IPluginBase implementation
    void setOwner(Firebird::IReferenceCounted* o) override;
    IReferenceCounted* getOwner() override;

    // IStreamPlugin implementation
    FB_BOOLEAN init(ThrowStatusWrapper* status, IAttachment* attachment) override;
    void finish(ThrowStatusWrapper* status) override;
    void startSegment(ThrowStatusWrapper* status, SegmentHeaderInfo* segmentHeader) override;
    void finishSegment(ThrowStatusWrapper* status) override;
    void startBlock(ThrowStatusWrapper* status, ISC_UINT64 blockOffset, unsigned blockLength) override;
    void setSegmentOffset(ISC_UINT64 offset) override;
    IStreamedTransaction* startTransaction(ThrowStatusWrapper* status, ISC_INT64 number) override;
    void setSequence(ThrowStatusWrapper* status, const char* name, ISC_INT64 value) override;
    FB_BOOLEAN matchTable(ThrowStatusWrapper* status, const char* relationName) override;
    IStreamedTransaction* getTransaction(ThrowStatusWrapper* status, ISC_INT64 number) override;
    void cleanupTransaction(ThrowStatusWrapper* status, ISC_INT64 number) override;
    void cleanupTransactions(ThrowStatusWrapper* status) override;
    void log(unsigned level, const char* message) override;

    std::string toUtf8(ThrowStatusWrapper* status, unsigned charsetId, std::string_view s);

    IUtil* getUtil() { return m_util; };

private:
    friend class SimpleJsonPluginTransaction;

    IMaster* m_master = nullptr;
    IConfig* m_config = nullptr;
    StringEncodeHelper m_stringEncoder;
    IStreamLogger* m_logger = nullptr;
    IReferenceCounted* m_owner = nullptr;
    std::atomic_int m_refCounter = 0;
    std::map<ISC_INT64, IStreamedTransaction*> m_transactions;
    IAttachment* m_att = nullptr;
    IUtil* m_util = nullptr;
    std::map<unsigned, StringConverterHelper> m_encodingConverters {};
    SegmentHeaderInfo m_segmentHeader;
    std::unique_ptr<std::regex> m_include_tables = nullptr;
    std::unique_ptr<std::regex> m_exclude_tables = nullptr;
    bool m_dumpBlobs = false;
    bool m_registerDDL = true;
    bool m_registerSequence = true;
    fs::path m_outputPath;

    class PluginImp;
    std::unique_ptr<PluginImp> pImp;
};

class SimpleJsonPluginTransaction final : public IStreamedTransactionImpl<SimpleJsonPluginTransaction, ThrowStatusWrapper> {
public:
    SimpleJsonPluginTransaction() = delete;
    SimpleJsonPluginTransaction(SimpleJsonStreamPlugin* applier, ISC_INT64 number);
    virtual ~SimpleJsonPluginTransaction();

    // IDisposable implementation
    void dispose() override;

    // IStreamedTransaction implementation
    void prepare(ThrowStatusWrapper* status) override;
    void commit(ThrowStatusWrapper* status) override;
    void rollback(ThrowStatusWrapper* status) override;
    void startSavepoint(ThrowStatusWrapper* status) override;
    void releaseSavepoint(ThrowStatusWrapper* status) override;
    void rollbackSavepoint(ThrowStatusWrapper* status) override;
    void insertRecord(ThrowStatusWrapper* status, const char* name, IStreamedRecord* record) override;
    void updateRecord(ThrowStatusWrapper* status, const char* name, IStreamedRecord* orgRecord, IStreamedRecord* newRecord) override;
    void deleteRecord(ThrowStatusWrapper* status, const char* name, IStreamedRecord* record) override;
    void executeSql(ThrowStatusWrapper* status, const char* sql) override;
    void executeSqlIntl(ThrowStatusWrapper* status, unsigned charset, const char* sql) override;
    void storeBlob(ThrowStatusWrapper* status, ISC_QUAD* blob_id,
        ISC_INT64 length, const unsigned char* data) override;

private:
    SimpleJsonStreamPlugin* m_streamPlugin = nullptr;
    ISC_INT64 m_number = 0;
};

} // namespace SimpleJsonPlugin

namespace {

struct vary {
    unsigned short vary_length;
    char vary_string[1]; /* CVC: The original declaration used UCHAR. */
};

constexpr const char* states[] = {
    "free",
    "used",
    "full",
    "archive"
};

void dumpRecord(ThrowStatusWrapper* status, SimpleJsonPlugin::SimpleJsonStreamPlugin* applier, IStreamedRecord* record, nlohmann::ordered_json& jRecord)
{
    using FbUtils::IscRandomStatus;

    for (unsigned i = 0; i < record->getCount(); i++) {
        auto field = record->getField(i);
        // For calculated fields, it may return null.
        if (!field)
            continue;
        auto fieldType = field->getType();
        // auto fieldSubType = field->getSubType();
        auto fieldCharsetId = field->getCharSet();
        std::string fieldName(field->getName());
        auto fieldScale = static_cast<short>(field->getScale());
        auto fieldLength = field->getLength();
        auto fieldData = field->getData();
        if (fieldData == nullptr) {
            jRecord[fieldName] = nullptr;
        } else {
            switch (fieldType) {
            case SQL_TEXT: {
                if (fieldCharsetId == CS_BINARY) {
                    const auto val = FbUtils::binary_to_hex(reinterpret_cast<const unsigned char*>(fieldData), fieldLength);
                    jRecord[fieldName] = val;
                } else {
                    const auto text = reinterpret_cast<const char*>(fieldData);
                    std::string_view s(text, fieldLength);
                    s = FbUtils::sv_rtrim_char(s, ' ');

                    if ((fieldCharsetId == CS_UTF8) || (fieldCharsetId == CS_NONE)) {
                        jRecord[fieldName] = s;
                    } else {
                        // character conversion required
                        const auto utf8Str = applier->toUtf8(status, fieldCharsetId, s);
                        jRecord[fieldName] = utf8Str;
                    }
                }
                break;
            }
            case SQL_VARYING: {

                const auto varchar = reinterpret_cast<const vary*>(fieldData);
                if (fieldCharsetId == CS_BINARY) {
                    const auto val = FbUtils::binary_to_hex(reinterpret_cast<const unsigned char*>(fieldData) + 2, varchar->vary_length);
                    jRecord[fieldName] = val;
                } else {
                    std::string_view s(varchar->vary_string, varchar->vary_length);
                    if ((fieldCharsetId == CS_UTF8) || (fieldCharsetId == CS_NONE)) {
                        jRecord[fieldName] = s;
                    } else {
                        // character conversion required
                        const auto val = applier->toUtf8(status, fieldCharsetId, s);
                        jRecord[fieldName] = val;
                    }
                }
                break;
            }
            case SQL_SHORT: {
                const auto value = *reinterpret_cast<const ISC_SHORT*>(fieldData);
                if (fieldScale == 0) {
                    jRecord[fieldName] = value;
                } else {
                    auto val = FbUtils::getScaledInteger(value, fieldScale);
                    jRecord[fieldName] = val;
                }
                break;
            }
            case SQL_LONG: {
                const auto value = *reinterpret_cast<const ISC_LONG*>(fieldData);
                if (fieldScale == 0) {
                    jRecord[fieldName] = static_cast<int32_t>(value);
                } else {
                    auto val = FbUtils::getScaledInteger(value, fieldScale);
                    jRecord[fieldName] = val;
                }
                break;
            }
            case SQL_INT64: {
                const auto value = *reinterpret_cast<const ISC_INT64*>(fieldData);
                if (fieldScale == 0) {
                    jRecord[fieldName] = static_cast<int64_t>(value);
                } else {
                    auto val = FbUtils::getScaledInteger(value, fieldScale);
                    jRecord[fieldName] = val;
                }
                break;
            }
            case SQL_INT128: {
                auto value = reinterpret_cast<const FB_I128*>(fieldData);
                const auto iInt128 = applier->getUtil()->getInt128(status);
                char buffer[IInt128::STRING_SIZE] = { ' ' };
                iInt128->toString(status, value, fieldScale, IInt128::STRING_SIZE, buffer);
                std::string_view s(buffer);
                const auto val = FbUtils::sv_rtrim_char(s, ' ');
                jRecord[fieldName] = val;
                break;
            }
            case SQL_FLOAT: {
                const auto value = *reinterpret_cast<const float*>(fieldData);
                jRecord[fieldName] = value;
                break;
            }
            case SQL_DOUBLE:
                [[fallthrough]];
            case SQL_D_FLOAT: {
                const auto value = *reinterpret_cast<const double*>(fieldData);
                jRecord[fieldName] = value;
                break;
            }
            case SQL_TIMESTAMP: {
                const auto value = *reinterpret_cast<const ISC_TIMESTAMP*>(fieldData);
                unsigned year = 0, month = 0, day = 0;
                unsigned hours = 0, minutes = 0, seconds = 0, fractions = 0;
                applier->getUtil()->decodeDate(value.timestamp_date, &year, &month, &day);
                applier->getUtil()->decodeTime(value.timestamp_time, &hours, &minutes, &seconds, &fractions);
                const auto val = FbUtils::vformat("%04d-%02d-%02d %02d:%02d:%02d.%d", year, month, day, hours, minutes, seconds, fractions);
                jRecord[fieldName] = val;
                break;
            }
            case SQL_TYPE_DATE: {
                const auto value = *reinterpret_cast<const ISC_DATE*>(fieldData);
                unsigned year = 0, month = 0, day = 0;
                applier->getUtil()->decodeDate(value, &year, &month, &day);
                const auto val = FbUtils::vformat("%04d-%02d-%02d", year, month, day);
                jRecord[fieldName] = val;
                break;
            }
            case SQL_TYPE_TIME: {
                const auto value = *reinterpret_cast<const ISC_TIME*>(fieldData);
                unsigned hours = 0, minutes = 0, seconds = 0, fractions = 0;
                applier->getUtil()->decodeTime(value, &hours, &minutes, &seconds, &fractions);
                const auto val = FbUtils::vformat("%02d:%02d:%02d.%d", hours, minutes, seconds, fractions);
                jRecord[fieldName] = val;
                break;
            }
            case SQL_TIMESTAMP_TZ: {
                const auto value = reinterpret_cast<const ISC_TIMESTAMP_TZ*>(fieldData);
                unsigned year = 0, month = 0, day = 0;
                unsigned hours = 0, minutes = 0, seconds = 0, fractions = 0;
                char timezoneBuffer[252] = { '\0' };
                applier->getUtil()->decodeTimeStampTz(status, value, &year, &month, &day, &hours, &minutes, &seconds, &fractions, 252, timezoneBuffer);
                const auto val = FbUtils::vformat("%04d-%02d-%02d %02d:%02d:%02d.%d %s", year, month, day, hours, minutes, seconds, fractions, timezoneBuffer);
                jRecord[fieldName] = val;
                break;
            }
            case SQL_TIME_TZ: {
                const auto value = reinterpret_cast<const ISC_TIME_TZ*>(fieldData);
                unsigned hours = 0, minutes = 0, seconds = 0, fractions = 0;
                char timezoneBuffer[252] = { '\0' };
                applier->getUtil()->decodeTimeTz(status, value, &hours, &minutes, &seconds, &fractions, 252, timezoneBuffer);
                const auto val = FbUtils::vformat("%02d:%02d:%02d.%d %s", hours, minutes, seconds, fractions, timezoneBuffer);
                jRecord[fieldName] = val;
                break;
            }
            case SQL_BOOLEAN: {
                const auto value = *reinterpret_cast<const FB_BOOLEAN*>(fieldData);
                const auto val = (value ? true : false);
                jRecord[fieldName] = val;
                break;
            }
            case SQL_DEC16: {
                auto value = reinterpret_cast<const FB_DEC16*>(fieldData);
                const auto iDecFloat16 = applier->getUtil()->getDecFloat16(status);
                char buffer[IDecFloat16::STRING_SIZE] = { ' ' };
                iDecFloat16->toString(status, value, IDecFloat16::STRING_SIZE, buffer);
                std::string_view s(buffer, IDecFloat16::STRING_SIZE);
                s = FbUtils::sv_rtrim_char(s);
                jRecord[fieldName] = s;
                break;
            }
            case SQL_DEC34: {
                auto value = reinterpret_cast<const FB_DEC34*>(fieldData);
                const auto iDecFloat34 = applier->getUtil()->getDecFloat34(status);
                char buffer[IDecFloat34::STRING_SIZE] = { ' ' };
                iDecFloat34->toString(status, value, IDecFloat34::STRING_SIZE, buffer);
                std::string_view s(buffer, IDecFloat34::STRING_SIZE);
                s = FbUtils::sv_rtrim_char(s);
                jRecord[fieldName] = s;
                break;
            }
            case SQL_BLOB: {
                const auto blobId = reinterpret_cast<const ISC_QUAD*>(fieldData);
                const auto val = FbUtils::vformat("%d:%d", blobId->gds_quad_high, blobId->gds_quad_low);
                jRecord[fieldName] = val;
                break;
            }
            case SQL_ARRAY: {
                IscRandomStatus statusVector("Array is not supported");
                throw Firebird::FbException(status, statusVector);
                break;
            }
            default: {
                IscRandomStatus statusVector("Unknown datatype");
                throw Firebird::FbException(status, statusVector);
            }
            }
        }
    }
}

} // namespace

namespace SimpleJsonPlugin {

class SimpleJsonStreamPlugin::PluginImp {
private:
    ordered_json doc;

public:
    PluginImp();
    void writeHeader(const SegmentHeaderInfo& headerInfo);
    void writeEvent(const ordered_json& event);
    void saveToFile(const fs::path& fileName);

    void setSequenceEvent(const char* name, ISC_INT64 value);

    void startTransactionEvent(ISC_INT64 number);
    void prepareTransactionEvent(ISC_INT64 number);
    void commitEvent(ISC_INT64 number);
    void rollbackEvent(ISC_INT64 number);
    void savepointEvent(ISC_INT64 number);
    void releaseSavepointEvent(ISC_INT64 number);
    void rollbackSavepointEvent(ISC_INT64 number);

    void executeSqlEvent(ISC_INT64 tnxNumber, const char* sql);

    void storeBlobEvent(ISC_INT64 tnxNumber, ISC_QUAD* blob_id,
        ISC_INT64 length, const unsigned char* data);

    void insertRecordEvent(ISC_INT64 tnxNumber, const char* name, const ordered_json& record);
    void updateRecordEvent(ISC_INT64 tnxNumber, const char* name, const ordered_json& orgRecord, const ordered_json& newRecord);
    void deleteRecordEvent(ISC_INT64 tnxNumber, const char* name, const ordered_json& record);
};

SimpleJsonStreamPlugin::PluginImp::PluginImp()
    : doc()
{
}

void SimpleJsonStreamPlugin::PluginImp::writeHeader(const SegmentHeaderInfo& headerInfo)
{
    // reset
    doc = {};

    doc["header"] = {};
    doc["events"] = ordered_json::array();

    ordered_json header;
    header["version"] = headerInfo.version;
    header["guid"] = headerInfo.guid;
    header["sequence"] = headerInfo.sequence;
    header["state"] = states[headerInfo.state];

    doc["header"] = header;
}

void SimpleJsonStreamPlugin::PluginImp::writeEvent(const ordered_json& event)
{
    doc["events"].push_back(event);
}

void SimpleJsonStreamPlugin::PluginImp::saveToFile(const fs::path& fileName)
{
    if (fs::exists(fileName)) {
        return;
    }
    std::ofstream o(fileName);
    o << std::setw(4) << doc << std::endl;
    o.close();

    // reset
    doc = {};
}

void SimpleJsonStreamPlugin::PluginImp::setSequenceEvent(const char* name, ISC_INT64 value)
{
    ordered_json jEvent;
    jEvent["event"] = "SET SEQUENCE";
    jEvent["sequence"] = name;
    jEvent["value"] = static_cast<int64_t>(value);

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::startTransactionEvent(ISC_INT64 number)
{
    ordered_json jEvent;
    jEvent["event"] = "START TRANSACTION";
    jEvent["tnx"] = static_cast<int64_t>(number);

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::prepareTransactionEvent(ISC_INT64 number)
{
    ordered_json jEvent;
    jEvent["event"] = "PREPARE TRANSACTION";
    jEvent["tnx"] = static_cast<int64_t>(number);

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::commitEvent(ISC_INT64 number)
{
    ordered_json jEvent;
    jEvent["event"] = "COMMIT";
    jEvent["tnx"] = static_cast<int64_t>(number);

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::rollbackEvent(ISC_INT64 number)
{
    ordered_json jEvent;
    jEvent["event"] = "ROLLBACK";
    jEvent["tnx"] = static_cast<int64_t>(number);

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::savepointEvent(ISC_INT64 number)
{
    ordered_json jEvent;
    jEvent["event"] = "SAVEPOINT";
    jEvent["tnx"] = static_cast<int64_t>(number);

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::releaseSavepointEvent(ISC_INT64 number)
{
    ordered_json jEvent;
    jEvent["event"] = "RELEASE SAVEPOINT";
    jEvent["tnx"] = static_cast<int64_t>(number);

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::rollbackSavepointEvent(ISC_INT64 number)
{
    ordered_json jEvent;
    jEvent["event"] = "ROLLBACK SAVEPOINT";
    jEvent["tnx"] = static_cast<int64_t>(number);

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::executeSqlEvent(ISC_INT64 tnxNumber, const char* sql)
{
    ordered_json jEvent;
    jEvent["event"] = "EXECUTE SQL";
    jEvent["sql"] = sql;
    jEvent["tnx"] = static_cast<int64_t>(tnxNumber);

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::storeBlobEvent(ISC_INT64 tnxNumber, ISC_QUAD* blob_id,
    ISC_INT64 length, const unsigned char* data)
{
    if ((length > 0) && (data != nullptr)) {
        auto bData = reinterpret_cast<const std::byte*>(data);
        std::vector<std::byte> blobData;
        blobData.insert(blobData.end(), bData, bData + length);
        auto binary = FbUtils::binary_to_hex(reinterpret_cast<unsigned char*>(blobData.data()), blobData.size());

        ordered_json jEvent;
        jEvent["event"] = "STORE BLOB";
        jEvent["blobId"] = FbUtils::vformat("%d:%d", blob_id->gds_quad_high, blob_id->gds_quad_low);
        jEvent["tnx"] = static_cast<int64_t>(tnxNumber);
        jEvent["data"] = binary;

        writeEvent(jEvent);
    }
}

void SimpleJsonStreamPlugin::PluginImp::insertRecordEvent(ISC_INT64 tnxNumber, const char* name, const ordered_json& record)
{
    ordered_json jEvent;
    jEvent["event"] = "INSERT";
    jEvent["table"] = name;
    jEvent["tnx"] = static_cast<int64_t>(tnxNumber);
    jEvent["record"] = record;

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::updateRecordEvent(ISC_INT64 tnxNumber, const char* name, const ordered_json& orgRecord, const ordered_json& newRecord)
{
    std::set<std::string> changedFields;
    for (const auto& [key, value] : newRecord.items()) {
        const auto iOldElem = orgRecord.find(key);
        if (iOldElem == orgRecord.cend()) {
            changedFields.insert(std::string(key));
        } else {
            const auto& oldVal = iOldElem.value();
            if (value != oldVal) {
                changedFields.insert(std::string(key));
            }
        }
    }

    ordered_json jEvent;
    jEvent["event"] = "UPDATE";
    jEvent["table"] = name;
    jEvent["tnx"] = static_cast<int64_t>(tnxNumber);
    jEvent["changedFields"] = changedFields;
    jEvent["oldRecord"] = orgRecord;
    jEvent["record"] = newRecord;

    writeEvent(jEvent);
}

void SimpleJsonStreamPlugin::PluginImp::deleteRecordEvent(ISC_INT64 tnxNumber, const char* name, const ordered_json& record)
{
    ordered_json jEvent;
    jEvent["event"] = "DELETE";
    jEvent["table"] = name;
    jEvent["tnx"] = static_cast<int64_t>(tnxNumber);
    jEvent["record"] = record;

    writeEvent(jEvent);
}

/////////////////////////////////////////
//
// SimpleJsonApplierPlugin implementation
//
/////////////////////////////////////////

SimpleJsonStreamPlugin::SimpleJsonStreamPlugin(IMaster* master, IConfig* config, IStringEncodeUtils* encodeUtils, IStreamLogger* logger)
    : m_master(master)
    , m_config(config)
    , m_stringEncoder(encodeUtils)
    , m_logger(logger)
    , m_owner(nullptr)
    , m_refCounter(0)
    , m_transactions()
    , m_att(nullptr)
    , m_util(master->getUtilInterface())
    , m_encodingConverters {}
    , m_segmentHeader()
    , m_include_tables(nullptr)
    , m_exclude_tables(nullptr)
    , m_dumpBlobs(false)
    , m_registerDDL(true)
    , m_registerSequence(true)
    , m_outputPath()
    , pImp(std::make_unique<PluginImp>())
{
    m_config->addRef();
}

SimpleJsonStreamPlugin::~SimpleJsonStreamPlugin()
{
    if (m_att)
        m_att->release();
    if (m_config)
        m_config->release();
}

void SimpleJsonStreamPlugin::addRef()
{
    ++m_refCounter;
}

int SimpleJsonStreamPlugin::release()
{
    if (--m_refCounter == 0) {
        delete this;
        return 0;
    }
    return 1;
}

void SimpleJsonStreamPlugin::setOwner(IReferenceCounted* o)
{
    m_owner = o;
}

IReferenceCounted* SimpleJsonStreamPlugin::getOwner()
{
    return m_owner;
}

FB_BOOLEAN SimpleJsonStreamPlugin::init(ThrowStatusWrapper* status, IAttachment* attachment)
{
    if (m_att) {
        m_att->release();
        m_att = nullptr;
    }
    m_att = attachment;
    if (m_att)
        m_att->addRef();

    AutoRelease<IConfigEntry> ceDumpBlobs(m_config->find(status, "dumpBlobs"));
    if (ceDumpBlobs) {
        m_dumpBlobs = ceDumpBlobs->getBoolValue();
    }

    AutoRelease<IConfigEntry> ceDDLEvents(m_config->find(status, "register_ddl_events"));
    if (ceDDLEvents) {
        m_registerDDL = ceDDLEvents->getBoolValue();
    }

    AutoRelease<IConfigEntry> ceSequenceEvents(m_config->find(status, "register_sequence_events"));
    if (ceSequenceEvents) {
        m_registerSequence = ceSequenceEvents->getBoolValue();
    }

    AutoRelease<IConfigEntry> ceOutputDir(m_config->find(status, "outputDir"));
    if (ceOutputDir) {
        m_outputPath.assign(ceOutputDir->getValue());
        if (!fs::is_directory(m_outputPath)) {
            auto statusVector = IscRandomStatus::createFmtStatus(R"(Output directory "%s" not found)", m_outputPath.generic_string().c_str());
            throw Firebird::FbException(status, statusVector);
        }
    } else {
        IscRandomStatus statusVector(R"(Parameter "outputDir" is required for plugin simple_json_plugin)");
        throw Firebird::FbException(status, statusVector);
    }

    AutoRelease<IConfigEntry> ceIncludeTables(m_config->find(status, "include_tables"));
    if (ceIncludeTables) {
        try {
            m_include_tables = std::make_unique<std::regex>(ceIncludeTables->getValue());
        } catch (const std::regex_error& e) {
            IscRandomStatus statusVector(e.what());
            throw Firebird::FbException(status, statusVector);
        }
    }

    AutoRelease<IConfigEntry> ceExcludeTables(m_config->find(status, "exclude_tables"));
    if (ceExcludeTables) {
        try {
            m_exclude_tables = std::make_unique<std::regex>(ceExcludeTables->getValue());
        } catch (const std::regex_error& e) {
            IscRandomStatus statusVector(e);
            throw Firebird::FbException(status, statusVector);
        }
    }

    return FB_TRUE;
}

void SimpleJsonStreamPlugin::finish([[maybe_unused]] ThrowStatusWrapper* status)
{
    m_include_tables = nullptr;
    m_exclude_tables = nullptr;
}

void SimpleJsonStreamPlugin::startSegment(ThrowStatusWrapper* status, SegmentHeaderInfo* segmentHeader)
try {
    // copy to internal segmentHeader
    m_segmentHeader.version = segmentHeader->version;
    m_segmentHeader.sequence = segmentHeader->sequence;
    m_segmentHeader.state = segmentHeader->state;
    m_segmentHeader.length = segmentHeader->length;
    memcpy(m_segmentHeader.name, segmentHeader->name, std::size(segmentHeader->name));
    memcpy(m_segmentHeader.guid, segmentHeader->guid, std::size(segmentHeader->guid));

    if (m_logger->getLevel() <= IStreamLogger::LEVEL_DEBUG) {
        // if the debug level is set, print the segment header
        std::stringstream ss;

        ss << std::endl;
        ss << std::setw(80) << std::setfill('=') << "=" << std::endl;
        ss << "Segment: " << m_segmentHeader.name << std::endl;
        ss << "Version: " << m_segmentHeader.version << std::endl;
        ss << "Guid: " << m_segmentHeader.guid << std::endl;
        ss << "Sequence: " << m_segmentHeader.sequence << std::endl;
        ss << "State: " << states[m_segmentHeader.state] << std::endl;
        ss << "Length: " << m_segmentHeader.length << std::endl;
        ss << std::setw(80) << std::setfill('=') << "=" << std::endl;
        ss << std::endl;

        m_logger->debug(ss.str().c_str());
    }

    pImp->writeHeader(m_segmentHeader);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonStreamPlugin::finishSegment(ThrowStatusWrapper* status)
try {
    std::string segmentName = m_segmentHeader.name;
    fs::path fileName = m_outputPath / (segmentName + ".json");

    pImp->saveToFile(fileName);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonStreamPlugin::startBlock([[maybe_unused]] ThrowStatusWrapper* status, [[maybe_unused]] ISC_UINT64 blockOffset, [[maybe_unused]] unsigned blockLength)
{}

void SimpleJsonStreamPlugin::setSegmentOffset([[maybe_unused]] ISC_UINT64 offset)
{}

IStreamedTransaction* SimpleJsonStreamPlugin::startTransaction(ThrowStatusWrapper* status, ISC_INT64 number)
try {
    auto tra = new SimpleJsonPluginTransaction(this, number);
    m_transactions[number] = tra;

    pImp->startTransactionEvent(number);

    return tra;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonStreamPlugin::setSequence(ThrowStatusWrapper* status, const char* name, ISC_INT64 value)
try {
    if (!m_registerSequence) {
        // If registration of the sequence value setting event is disabled, then exit.
        return;
    }

    pImp->setSequenceEvent(name, value);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

FB_BOOLEAN SimpleJsonStreamPlugin::matchTable(ThrowStatusWrapper* status, const char* relationName)
try {
    bool match = true;
    if (m_include_tables != nullptr) {
        // The table name must match the regular expression
        match = match && std::regex_match(relationName, *m_include_tables);
    }
    if (m_exclude_tables != nullptr) {
        // Table name must not match regular expression
        match = match && !std::regex_match(relationName, *m_exclude_tables);
    }
    return match ? FB_TRUE : FB_FALSE;
} catch (const std::regex_error& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

IStreamedTransaction* SimpleJsonStreamPlugin::getTransaction(ThrowStatusWrapper* status, ISC_INT64 number)
try {
    const auto it = m_transactions.find(number);
    if (it == m_transactions.end()) {
        auto statusVector = IscRandomStatus::createFmtStatus("Transaction %" UQUADFORMAT " not found, segment name %s", number, m_segmentHeader.name);
        throw Firebird::FbException(status, statusVector);
    }
    auto tra = (*it).second;
    return tra;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonStreamPlugin::cleanupTransaction(ThrowStatusWrapper* status, ISC_INT64 number)
try {
    m_transactions.erase(number);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonStreamPlugin::cleanupTransactions(ThrowStatusWrapper* status)
try {
    // get a list of transaction numbers
    std::vector<ISC_INT64> numbers;
    std::transform(
        std::begin(m_transactions),
        std::end(m_transactions),
        std::back_inserter(numbers),
        [](auto const& pair) {
            return pair.first;
        });
    // rollback all transactions
    for (auto traNum : numbers) {
        auto tra = m_transactions[traNum];
        tra->rollback(status);
        tra->dispose();
        m_transactions.erase(traNum);
    }
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonStreamPlugin::log(unsigned level, const char* message)
{
    m_logger->log(level, message);
}

std::string SimpleJsonStreamPlugin::toUtf8(ThrowStatusWrapper* status, unsigned charsetId, std::string_view s)
try {
    auto [it, result] = m_encodingConverters.try_emplace(
        charsetId,
        lazy_convert_construct([charsetId, status, this] {
            return m_stringEncoder.getConverterById(status, charsetId);
        })
    );
    return it->second.toUtf8(status, s);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw FbException(status, statusVector);
}

SimpleJsonPluginTransaction::SimpleJsonPluginTransaction(SimpleJsonStreamPlugin* applier, ISC_INT64 number)
    : m_streamPlugin(applier)
    , m_number(number)
{
    m_streamPlugin->addRef(); // Lock parent from disappearing
}

SimpleJsonPluginTransaction::~SimpleJsonPluginTransaction()
{
    m_streamPlugin->release();
}

void SimpleJsonPluginTransaction::dispose()
{
    delete this;
}

void SimpleJsonPluginTransaction::prepare(ThrowStatusWrapper* status)
try {
    m_streamPlugin->pImp->prepareTransactionEvent(m_number);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonPluginTransaction::commit(ThrowStatusWrapper* status)
try {
    m_streamPlugin->pImp->commitEvent(m_number);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonPluginTransaction::rollback(ThrowStatusWrapper* status)
try {
    m_streamPlugin->pImp->rollbackEvent(m_number);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonPluginTransaction::startSavepoint(ThrowStatusWrapper* status)
try {
    m_streamPlugin->pImp->savepointEvent(m_number);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonPluginTransaction::releaseSavepoint(ThrowStatusWrapper* status)
try {
    m_streamPlugin->pImp->releaseSavepointEvent(m_number);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonPluginTransaction::rollbackSavepoint(ThrowStatusWrapper* status)
try {
    m_streamPlugin->pImp->rollbackSavepointEvent(m_number);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonPluginTransaction::insertRecord(ThrowStatusWrapper* status, const char* name, IStreamedRecord* record)
try {
    if (record->getCount() == 0) {
        m_streamPlugin->m_logger->info(FbUtils::vformat("INSERT %s", name).c_str());
        std::string msg = FbUtils::vformat(R"(Format not found. Segment name %s)", m_streamPlugin->m_segmentHeader.name);
        m_streamPlugin->m_logger->warning(msg.c_str());
        return;
    }
    m_streamPlugin->m_logger->debug(FbUtils::vformat("[%" UQUADFORMAT "] INSERT %s (length: %d)", m_number, name, record->getRawLength()).c_str());

    ordered_json jRecord;

    dumpRecord(status, m_streamPlugin, record, jRecord);

    m_streamPlugin->pImp->insertRecordEvent(m_number, name, jRecord);

} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonPluginTransaction::updateRecord(ThrowStatusWrapper* status, const char* name, IStreamedRecord* orgRecord, IStreamedRecord* newRecord)
try {
    if (orgRecord->getCount() == 0) {
        m_streamPlugin->m_logger->info(FbUtils::vformat("UPDATE %s", name).c_str());
        std::string msg = FbUtils::vformat(R"(No format found for old record table. Segment name %s)", m_streamPlugin->m_segmentHeader.name);
        m_streamPlugin->m_logger->warning(msg.c_str());
        return;
    }
    if (newRecord->getCount() == 0) {
        m_streamPlugin->m_logger->info(FbUtils::vformat("UPDATE %s", name).c_str());
        std::string msg = FbUtils::vformat(R"(No format found for new record table. Segment name %s)", m_streamPlugin->m_segmentHeader.name);
        m_streamPlugin->m_logger->warning(msg.c_str());
        return;
    }
    m_streamPlugin->m_logger->debug(FbUtils::vformat("[%" UQUADFORMAT "] UPDATE %s (orgLength: %d, newLength: %d)", m_number, name, orgRecord->getRawLength(), newRecord->getRawLength()).c_str());

    ordered_json jOrgRecord;
    ordered_json jNewRecord;

    dumpRecord(status, m_streamPlugin, orgRecord, jOrgRecord);
    dumpRecord(status, m_streamPlugin, newRecord, jNewRecord);

    m_streamPlugin->pImp->updateRecordEvent(m_number, name, jOrgRecord, jNewRecord);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonPluginTransaction::deleteRecord(ThrowStatusWrapper* status, const char* name, IStreamedRecord* record)
try {
    if (record->getCount() == 0) {
        m_streamPlugin->m_logger->info(FbUtils::vformat("DELETE %s", name).c_str());
        std::string msg = FbUtils::vformat(R"(Format not found. Segment name %s)", m_streamPlugin->m_segmentHeader.name);
        m_streamPlugin->m_logger->warning(msg.c_str());
        return;
    }
    m_streamPlugin->m_logger->debug(FbUtils::vformat("[%" UQUADFORMAT "] DELETE %s (length: %d)", m_number, name, record->getRawLength()).c_str());

    ordered_json jRecord;

    dumpRecord(status, m_streamPlugin, record, jRecord);

    m_streamPlugin->pImp->deleteRecordEvent(m_number, name, jRecord);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonPluginTransaction::executeSql(ThrowStatusWrapper* status, const char* sql)
try {
    if (!m_streamPlugin->m_registerDDL) {
        // If registration of DDL events is disabled, exit.
        return;
    }

    m_streamPlugin->pImp->executeSqlEvent(m_number, sql);

} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

void SimpleJsonPluginTransaction::executeSqlIntl(ThrowStatusWrapper* status, unsigned charset, const char* sql)
{
    if (!m_streamPlugin->m_registerDDL) {
        // If registration of DDL events is disabled, exit.
        return;
    }

    const auto utf8Sql = FbUtils::rtrim(m_streamPlugin->toUtf8(status, charset, sql));
    executeSql(status, utf8Sql.c_str());
}

void SimpleJsonPluginTransaction::storeBlob(ThrowStatusWrapper* status, ISC_QUAD* blob_id,
    ISC_INT64 length, const unsigned char* data)
try {
    if (!m_streamPlugin->m_dumpBlobs) {
        // If the BLOB dump is disabled, then exit. This will save memory consumption.
        return;
    }

    m_streamPlugin->pImp->storeBlobEvent(m_number, blob_id, length, data);
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

SimpleJsonPluginFactory::SimpleJsonPluginFactory(IMaster* master)
    : m_master(master)
{
}

IStreamPlugin* SimpleJsonPluginFactory::createPlugin(ThrowStatusWrapper* status, IConfig* config, IStringEncodeUtils* encodeUtils, IStreamLogger* logger)
try {
    auto plugin = new SimpleJsonStreamPlugin(m_master, config, encodeUtils, logger);
    plugin->addRef();
    return plugin;
} catch (const std::exception& e) {
    IscRandomStatus statusVector(e);
    throw Firebird::FbException(status, statusVector);
}

} // namespace SimpleJsonPlugin
