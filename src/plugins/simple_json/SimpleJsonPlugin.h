#pragma once
#ifndef SIMPLE_JSON_PLUGIN_H
#define SIMPLE_JSON_PLUGIN_H

#include <map>
#include <set>
#include <vector>
#include <list>
#include <atomic>
#include <memory>
#include <stack>
#include <regex>

#include <nlohmann/json.hpp>
#include "../../include/ReplicateApplierInterface.h"
#include "../../common/EncodeUtils.h"

using namespace Firebird;

using json = nlohmann::json;

namespace SimpleJsonPlugin
{
	using FBStringEncoderPtr = std::unique_ptr<FBStringEncoder>;

	using EventList = std::list<json>;
	using EventListPtr = std::unique_ptr<EventList>;


	class SimpleJsonPlugin final : public IReplicateApplierPluginImpl<SimpleJsonPlugin, ThrowStatusWrapper>
	{
	public:
		SimpleJsonPlugin() = delete;
		SimpleJsonPlugin(IMaster* master, IConfig* config, IStreamLogger* logger);
		virtual ~SimpleJsonPlugin();

		// IReferenceCounted implementation
		void addRef() override;
		int release() override;

		// IPluginBase implementation
		void setOwner(IReferenceCounted* o) override;
		IReferenceCounted* getOwner() override;

		// IReplicateApplier implementation
		FB_BOOLEAN init(ThrowStatusWrapper* status, IAttachment* attachment) override;
		void finish(ThrowStatusWrapper* status) override;
		void startSegment(ThrowStatusWrapper* status, SegmentHeaderInfo* segmentHeader) override;
		void finishSegment(ThrowStatusWrapper* status) override;
		IApplierTransaction* startTransaction(ThrowStatusWrapper* status, ISC_INT64 number) override;
		void setSequence(ThrowStatusWrapper* status, const char* name, ISC_INT64 value) override;
		FB_BOOLEAN matchTable(ThrowStatusWrapper* status, const char* relationName) override;
		IApplierTransaction* getTransaction(ThrowStatusWrapper* status, ISC_INT64 number) override;
		void cleanupTransaction(ThrowStatusWrapper* status, ISC_INT64 number) override;
		void cleanupTransactions(ThrowStatusWrapper* status) override;
		void log(unsigned level, const char* message) override;

		const FBStringEncoderPtr& getEncoder(unsigned charsetId);
	private:
		friend class ReplTransaction;

		IMaster* m_master = nullptr;
		IConfig* m_config = nullptr;
		IStreamLogger* m_logger = nullptr;
		IReferenceCounted* m_owner = nullptr;
		std::atomic_int m_refCounter = 0;
		map<ISC_INT64, IApplierTransaction*> m_transactions;
		IAttachment* m_att = nullptr;
		IUtil* m_util = nullptr;
		std::map<unsigned, FBStringEncoderPtr> m_encoders;
		SegmentHeaderInfo m_segmentHeader;	
		unique_ptr<std::regex> m_include_tables = nullptr;
		unique_ptr<std::regex> m_exclude_tables = nullptr;
		bool m_dumpBlobs = false;
		bool m_registerDDL = true;
		bool m_registerSequence = true;

		class PluginImp;
//		unique_ptr<PluginImp> pImp = nullptr;
	};

	class ReplTransaction final : public IApplierTransactionImpl<ReplTransaction, ThrowStatusWrapper>
	{
	public:
		ReplTransaction() = delete;
		ReplTransaction(SimpleJsonPlugin* applier, ISC_INT64 number);
		virtual ~ReplTransaction();

		// IDisposable implementation
		void dispose() override;

		// IReplicatedTransaction implementation
		void prepare(ThrowStatusWrapper* status) override;
		void commit(ThrowStatusWrapper* status) override;
		void rollback(ThrowStatusWrapper* status) override;
		void startSavepoint(ThrowStatusWrapper* status) override;
		void releaseSavepoint(ThrowStatusWrapper* status) override;
		void rollbackSavepoint(ThrowStatusWrapper* status) override;
		void insertRecord(ThrowStatusWrapper* status, const char* name, IReplicatedRecord* record) override;
		void updateRecord(ThrowStatusWrapper* status, const char* name, IReplicatedRecord* orgRecord, IReplicatedRecord* newRecord) override;
		void deleteRecord(ThrowStatusWrapper* status, const char* name, IReplicatedRecord* record) override;
		void executeSql(ThrowStatusWrapper* status, const char* sql) override;
		void executeSqlIntl(ThrowStatusWrapper* status, unsigned charset, const char* sql) override;

		// IApplierTransaction implementation
		void storeBlob(ThrowStatusWrapper* status, ISC_QUAD* blob_id,
			ISC_INT64 length, const unsigned char* data) override;
	private:
		void dumpRecord(ThrowStatusWrapper* status, IReplicatedRecord* record, json& jRecord, bool dumpBlob = false, json* const jBlobs = nullptr);
	private:
		SimpleJsonPlugin* m_applier = nullptr;
		ISC_INT64 m_number = 0;
		stack<EventListPtr> m_savepoints {};
		std::map<ISC_INT64, std::vector<std::byte>> m_blobs {};
	};

	class SimpleJsonPluginFactory final : public IReplicateApplierFactoryImpl<SimpleJsonPluginFactory, ThrowStatusWrapper>
	{
	private:
		IMaster* m_master = nullptr;
	public:
		SimpleJsonPluginFactory() = delete;
		explicit SimpleJsonPluginFactory(IMaster* master);

		IReplicateApplierPlugin* createPlugin(ThrowStatusWrapper* status, IConfig* config, IStreamLogger* logger);
	};

};

#endif
