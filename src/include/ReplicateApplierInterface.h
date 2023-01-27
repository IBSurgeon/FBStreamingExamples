#pragma once
#ifndef REPLICATE_APPLIER_H
#define REPLICATE_APPLIER_H

#ifdef _WINDOWS
#pragma warning(push)
#pragma warning(disable:4244)
#endif
#include "firebird/Interface.h"
#ifdef _WINDOWS
#pragma warning(pop)
#endif

namespace Firebird
{

	class IApplierTransaction;
	class IReplicateApplierPlugin;
	class IStreamLogger;
	class IReplicateApplierFactory;
	class IStreamPluginManager;


	struct SegmentHeaderInfo
	{
		ISC_USHORT version;
		ISC_USHORT state;
		char guid[39];
		ISC_UINT64 sequence;
		ISC_UINT64 length;
		char name[255];
	};

	class IApplierTransaction : public IReplicatedTransaction
	{
	public:
		struct VTable : public IReplicatedTransaction::VTable
		{
			void (CLOOP_CARG* storeBlob)(IApplierTransaction* self, IStatus* status, ISC_QUAD* blob_id, ISC_INT64 length, const unsigned char* data) throw();
		};

	protected:
		IApplierTransaction(DoNotInherit)
			: IReplicatedTransaction(DoNotInherit())
		{
		}

		~IApplierTransaction()
		{
		}

	public:
		static const unsigned VERSION = 4;

		template <typename StatusType> void storeBlob(StatusType* status, ISC_QUAD* blob_id, ISC_INT64 length, const unsigned char* data)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->storeBlob(this, status, blob_id, length, data);
			StatusType::checkException(status);
		}
	};

	class IReplicateApplierPlugin : public IPluginBase
	{
	public:
		struct VTable : public IPluginBase::VTable
		{
			FB_BOOLEAN (CLOOP_CARG *init)(IReplicateApplierPlugin* self, IStatus* status, IAttachment* attachment) throw();
			void (CLOOP_CARG *finish)(IReplicateApplierPlugin* self, IStatus* status) throw();
			void (CLOOP_CARG *startSegment)(IReplicateApplierPlugin* self, IStatus* status, SegmentHeaderInfo* segmentHeader) throw();
			void (CLOOP_CARG *finishSegment)(IReplicateApplierPlugin* self, IStatus* status) throw();
			IApplierTransaction* (CLOOP_CARG *startTransaction)(IReplicateApplierPlugin* self, IStatus* status, ISC_INT64 number) throw();
			void (CLOOP_CARG *setSequence)(IReplicateApplierPlugin* self, IStatus* status, const char* name, ISC_INT64 value) throw();
			FB_BOOLEAN (CLOOP_CARG *matchTable)(IReplicateApplierPlugin* self, IStatus* status, const char* relationName) throw();
			IApplierTransaction* (CLOOP_CARG *getTransaction)(IReplicateApplierPlugin* self, IStatus* status, ISC_INT64 number) throw();
			void (CLOOP_CARG *cleanupTransaction)(IReplicateApplierPlugin* self, IStatus* status, ISC_INT64 number) throw();
			void (CLOOP_CARG *cleanupTransactions)(IReplicateApplierPlugin* self, IStatus* status) throw();
			void (CLOOP_CARG *log)(IReplicateApplierPlugin* self, unsigned level, const char* message) throw();
		};

	protected:
		IReplicateApplierPlugin(DoNotInherit)
			: IPluginBase(DoNotInherit())
		{
		}

		~IReplicateApplierPlugin()
		{
		}

	public:
		static const unsigned VERSION = 4;

		template <typename StatusType> FB_BOOLEAN init(StatusType* status, IAttachment* attachment)
		{
			StatusType::clearException(status);
			FB_BOOLEAN ret = static_cast<VTable*>(this->cloopVTable)->init(this, status, attachment);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> void finish(StatusType* status)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->finish(this, status);
			StatusType::checkException(status);
		}

		template <typename StatusType> void startSegment(StatusType* status, SegmentHeaderInfo* segmentHeader)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->startSegment(this, status, segmentHeader);
			StatusType::checkException(status);
		}

		template <typename StatusType> void finishSegment(StatusType* status)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->finishSegment(this, status);
			StatusType::checkException(status);
		}

		template <typename StatusType> IApplierTransaction* startTransaction(StatusType* status, ISC_INT64 number)
		{
			StatusType::clearException(status);
			IApplierTransaction* ret = static_cast<VTable*>(this->cloopVTable)->startTransaction(this, status, number);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> void setSequence(StatusType* status, const char* name, ISC_INT64 value)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->setSequence(this, status, name, value);
			StatusType::checkException(status);
		}

		template <typename StatusType> FB_BOOLEAN matchTable(StatusType* status, const char* relationName)
		{
			StatusType::clearException(status);
			FB_BOOLEAN ret = static_cast<VTable*>(this->cloopVTable)->matchTable(this, status, relationName);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> IApplierTransaction* getTransaction(StatusType* status, ISC_INT64 number)
		{
			StatusType::clearException(status);
			IApplierTransaction* ret = static_cast<VTable*>(this->cloopVTable)->getTransaction(this, status, number);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> void cleanupTransaction(StatusType* status, ISC_INT64 number)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->cleanupTransaction(this, status, number);
			StatusType::checkException(status);
		}

		template <typename StatusType> void cleanupTransactions(StatusType* status)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->cleanupTransactions(this, status);
			StatusType::checkException(status);
		}

		void log(unsigned level, const char* message)
		{
			static_cast<VTable*>(this->cloopVTable)->log(this, level, message);
		}
	};

	class IStreamLogger : public IReferenceCounted
	{
	public:
		struct VTable : public IReferenceCounted::VTable
		{
			unsigned (CLOOP_CARG* getLevel)(IStreamLogger* self) throw();
			void (CLOOP_CARG* log)(IStreamLogger* self, unsigned level, const char* message) throw();
			void (CLOOP_CARG* trace)(IStreamLogger* self, const char* message) throw();
			void (CLOOP_CARG* debug)(IStreamLogger* self, const char* message) throw();
			void (CLOOP_CARG* info)(IStreamLogger* self, const char* message) throw();
			void (CLOOP_CARG* warning)(IStreamLogger* self, const char* message) throw();
			void (CLOOP_CARG* error)(IStreamLogger* self, const char* message) throw();
			void (CLOOP_CARG* critical)(IStreamLogger* self, const char* message) throw();
		};

	protected:
		IStreamLogger(DoNotInherit)
			: IReferenceCounted(DoNotInherit())
		{
		}

		~IStreamLogger()
		{
		}

	public:
		static const unsigned VERSION = 3;

		static const unsigned LEVEL_TRACE = 0;
		static const unsigned LEVEL_DEBUG = 1;
		static const unsigned LEVEL_INFO = 2;
		static const unsigned LEVEL_WARN = 3;
		static const unsigned LEVEL_ERROR = 4;
		static const unsigned LEVEL_CRITICAL = 5;
		static const unsigned LEVEL_OFF = 6;

		unsigned getLevel()
		{
			unsigned ret = static_cast<VTable*>(this->cloopVTable)->getLevel(this);
			return ret;
		}

		void log(unsigned level, const char* message)
		{
			static_cast<VTable*>(this->cloopVTable)->log(this, level, message);
		}

		void trace(const char* message)
		{
			static_cast<VTable*>(this->cloopVTable)->trace(this, message);
		}

		void debug(const char* message)
		{
			static_cast<VTable*>(this->cloopVTable)->debug(this, message);
		}

		void info(const char* message)
		{
			static_cast<VTable*>(this->cloopVTable)->info(this, message);
		}

		void warning(const char* message)
		{
			static_cast<VTable*>(this->cloopVTable)->warning(this, message);
		}

		void error(const char* message)
		{
			static_cast<VTable*>(this->cloopVTable)->error(this, message);
		}

		void critical(const char* message)
		{
			static_cast<VTable*>(this->cloopVTable)->critical(this, message);
		}
	};

	class IReplicateApplierFactory : public IVersioned
	{
	public:
		struct VTable : public IVersioned::VTable
		{
			IReplicateApplierPlugin* (CLOOP_CARG* createPlugin)(IReplicateApplierFactory* self, IStatus* status, IConfig* config, IStreamLogger* logger) throw();
		};

	protected:
		IReplicateApplierFactory(DoNotInherit)
			: IVersioned(DoNotInherit())
		{
		}

		~IReplicateApplierFactory()
		{
		}

	public:
		static const unsigned VERSION = 2;

		template <typename StatusType> IReplicateApplierPlugin* createPlugin(StatusType* status, IConfig* config, IStreamLogger* logger)
		{
			StatusType::clearException(status);
			IReplicateApplierPlugin* ret = static_cast<VTable*>(this->cloopVTable)->createPlugin(this, status, config, logger);
			StatusType::checkException(status);
			return ret;
		}
	};

	class IStreamPluginManager : public IDisposable
	{
	public:
		struct VTable : public IDisposable::VTable
		{
			void (CLOOP_CARG* registerPluginFactory)(IStreamPluginManager* self, const char* pluginName, IReplicateApplierFactory* factory) throw();
			IReplicateApplierPlugin* (CLOOP_CARG* getPlugin)(IStreamPluginManager* self, IStatus* status, const char* pluginName, IConfig* config, IStreamLogger* logger) throw();
		};

	protected:
		IStreamPluginManager(DoNotInherit)
			: IDisposable(DoNotInherit())
		{
		}

		~IStreamPluginManager()
		{
		}

	public:
		static const unsigned VERSION = 3;

		void registerPluginFactory(const char* pluginName, IReplicateApplierFactory* factory)
		{
			static_cast<VTable*>(this->cloopVTable)->registerPluginFactory(this, pluginName, factory);
		}

		template <typename StatusType> IReplicateApplierPlugin* getPlugin(StatusType* status, const char* pluginName, IConfig* config, IStreamLogger* logger)
		{
			StatusType::clearException(status);
			IReplicateApplierPlugin* ret = static_cast<VTable*>(this->cloopVTable)->getPlugin(this, status, pluginName, config, logger);
			StatusType::checkException(status);
			return ret;
		}
	};

	template <typename Name, typename StatusType, typename Base>
	class IApplierTransactionBaseImpl : public Base
	{
	public:
		typedef IApplierTransaction Declaration;

		IApplierTransactionBaseImpl(DoNotInherit = DoNotInherit())
		{
			static struct VTableImpl : Base::VTable
			{
				VTableImpl()
				{
					this->version = Base::VERSION;
					this->dispose = &Name::cloopdisposeDispatcher;
					this->prepare = &Name::cloopprepareDispatcher;
					this->commit = &Name::cloopcommitDispatcher;
					this->rollback = &Name::clooprollbackDispatcher;
					this->startSavepoint = &Name::cloopstartSavepointDispatcher;
					this->releaseSavepoint = &Name::cloopreleaseSavepointDispatcher;
					this->rollbackSavepoint = &Name::clooprollbackSavepointDispatcher;
					this->insertRecord = &Name::cloopinsertRecordDispatcher;
					this->updateRecord = &Name::cloopupdateRecordDispatcher;
					this->deleteRecord = &Name::cloopdeleteRecordDispatcher;
					this->executeSql = &Name::cloopexecuteSqlDispatcher;
					this->executeSqlIntl = &Name::cloopexecuteSqlIntlDispatcher;
					this->storeBlob = &Name::cloopstoreBlobDispatcher;
				}
			} vTable;

			this->cloopVTable = &vTable;
		}

		static void CLOOP_CARG cloopstoreBlobDispatcher(IApplierTransaction* self, IStatus* status, ISC_QUAD* blob_id, ISC_INT64 length, const unsigned char* data) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::storeBlob(&status2, blob_id, length, data);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopprepareDispatcher(IReplicatedTransaction* self, IStatus* status) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::prepare(&status2);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopcommitDispatcher(IReplicatedTransaction* self, IStatus* status) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::commit(&status2);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG clooprollbackDispatcher(IReplicatedTransaction* self, IStatus* status) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::rollback(&status2);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopstartSavepointDispatcher(IReplicatedTransaction* self, IStatus* status) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::startSavepoint(&status2);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopreleaseSavepointDispatcher(IReplicatedTransaction* self, IStatus* status) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::releaseSavepoint(&status2);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG clooprollbackSavepointDispatcher(IReplicatedTransaction* self, IStatus* status) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::rollbackSavepoint(&status2);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopinsertRecordDispatcher(IReplicatedTransaction* self, IStatus* status, const char* name, IReplicatedRecord* record) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::insertRecord(&status2, name, record);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopupdateRecordDispatcher(IReplicatedTransaction* self, IStatus* status, const char* name, IReplicatedRecord* orgRecord, IReplicatedRecord* newRecord) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::updateRecord(&status2, name, orgRecord, newRecord);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopdeleteRecordDispatcher(IReplicatedTransaction* self, IStatus* status, const char* name, IReplicatedRecord* record) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::deleteRecord(&status2, name, record);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopexecuteSqlDispatcher(IReplicatedTransaction* self, IStatus* status, const char* sql) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::executeSql(&status2, sql);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopexecuteSqlIntlDispatcher(IReplicatedTransaction* self, IStatus* status, unsigned charset, const char* sql) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::executeSqlIntl(&status2, charset, sql);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopdisposeDispatcher(IDisposable* self) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::dispose();
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}
	};

	template <typename Name, typename StatusType, typename Base = IReplicatedTransactionImpl<Name, StatusType, Inherit<IDisposableImpl<Name, StatusType, Inherit<IVersionedImpl<Name, StatusType, Inherit<IApplierTransaction> > > > > > >
	class IApplierTransactionImpl : public IApplierTransactionBaseImpl<Name, StatusType, Base>
	{
	protected:
		IApplierTransactionImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IApplierTransactionImpl()
		{
		}

		virtual void storeBlob(StatusType* status, ISC_QUAD* blob_id, ISC_INT64 length, const unsigned char* data) = 0;
	};

	template <typename Name, typename StatusType, typename Base>
	class IReplicateApplierPluginBaseImpl : public Base
	{
	public:
		typedef IReplicateApplierPlugin Declaration;

		IReplicateApplierPluginBaseImpl(DoNotInherit = DoNotInherit())
		{
			static struct VTableImpl : Base::VTable
			{
				VTableImpl()
				{
					this->version = Base::VERSION;
					this->addRef = &Name::cloopaddRefDispatcher;
					this->release = &Name::cloopreleaseDispatcher;
					this->setOwner = &Name::cloopsetOwnerDispatcher;
					this->getOwner = &Name::cloopgetOwnerDispatcher;
					this->init = &Name::cloopinitDispatcher;
					this->finish = &Name::cloopfinishDispatcher;
					this->startSegment = &Name::cloopstartSegmentDispatcher;
					this->finishSegment = &Name::cloopfinishSegmentDispatcher;
					this->startTransaction = &Name::cloopstartTransactionDispatcher;
					this->setSequence = &Name::cloopsetSequenceDispatcher;
					this->matchTable = &Name::cloopmatchTableDispatcher;
					this->getTransaction = &Name::cloopgetTransactionDispatcher;
					this->cleanupTransaction = &Name::cloopcleanupTransactionDispatcher;
					this->cleanupTransactions = &Name::cloopcleanupTransactionsDispatcher;
					this->log = &Name::clooplogDispatcher;
				}
			} vTable;

			this->cloopVTable = &vTable;
		}

		static FB_BOOLEAN CLOOP_CARG cloopinitDispatcher(IReplicateApplierPlugin* self, IStatus* status, IAttachment* attachment) throw()
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::init(&status2, attachment);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<FB_BOOLEAN>(0);
			}
		}

		static void CLOOP_CARG cloopfinishDispatcher(IReplicateApplierPlugin* self, IStatus* status) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::finish(&status2);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopstartSegmentDispatcher(IReplicateApplierPlugin* self, IStatus* status, SegmentHeaderInfo* segmentHeader) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::startSegment(&status2, segmentHeader);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopfinishSegmentDispatcher(IReplicateApplierPlugin* self, IStatus* status) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::finishSegment(&status2);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static IApplierTransaction* CLOOP_CARG cloopstartTransactionDispatcher(IReplicateApplierPlugin* self, IStatus* status, ISC_INT64 number) throw()
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::startTransaction(&status2, number);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<IApplierTransaction*>(0);
			}
		}

		static void CLOOP_CARG cloopsetSequenceDispatcher(IReplicateApplierPlugin* self, IStatus* status, const char* name, ISC_INT64 value) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::setSequence(&status2, name, value);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static FB_BOOLEAN CLOOP_CARG cloopmatchTableDispatcher(IReplicateApplierPlugin* self, IStatus* status, const char* relationName) throw()
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::matchTable(&status2, relationName);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<FB_BOOLEAN>(0);
			}
		}

		static IApplierTransaction* CLOOP_CARG cloopgetTransactionDispatcher(IReplicateApplierPlugin* self, IStatus* status, ISC_INT64 number) throw()
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::getTransaction(&status2, number);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<IApplierTransaction*>(0);
			}
		}

		static void CLOOP_CARG cloopcleanupTransactionDispatcher(IReplicateApplierPlugin* self, IStatus* status, ISC_INT64 number) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::cleanupTransaction(&status2, number);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopcleanupTransactionsDispatcher(IReplicateApplierPlugin* self, IStatus* status) throw()
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::cleanupTransactions(&status2);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG clooplogDispatcher(IReplicateApplierPlugin* self, unsigned level, const char* message) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::log(level, message);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static void CLOOP_CARG cloopsetOwnerDispatcher(IPluginBase* self, IReferenceCounted* r) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::setOwner(r);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static IReferenceCounted* CLOOP_CARG cloopgetOwnerDispatcher(IPluginBase* self) throw()
		{
			try
			{
				return static_cast<Name*>(self)->Name::getOwner();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<IReferenceCounted*>(0);
			}
		}

		static void CLOOP_CARG cloopaddRefDispatcher(IReferenceCounted* self) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::addRef();
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static int CLOOP_CARG cloopreleaseDispatcher(IReferenceCounted* self) throw()
		{
			try
			{
				return static_cast<Name*>(self)->Name::release();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<int>(0);
			}
		}
	};

	template <typename Name, typename StatusType, typename Base = IPluginBaseImpl<Name, StatusType, Inherit<IReferenceCountedImpl<Name, StatusType, Inherit<IVersionedImpl<Name, StatusType, Inherit<IReplicateApplierPlugin> > > > > > >
	class IReplicateApplierPluginImpl : public IReplicateApplierPluginBaseImpl<Name, StatusType, Base>
	{
	protected:
		IReplicateApplierPluginImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IReplicateApplierPluginImpl()
		{
		}

		virtual FB_BOOLEAN init(StatusType* status, IAttachment* attachment) = 0;
		virtual void finish(StatusType* status) = 0;
		virtual void startSegment(StatusType* status, SegmentHeaderInfo* segmentHeader) = 0;
		virtual void finishSegment(StatusType* status) = 0;
		virtual IApplierTransaction* startTransaction(StatusType* status, ISC_INT64 number) = 0;
		virtual void setSequence(StatusType* status, const char* name, ISC_INT64 value) = 0;
		virtual FB_BOOLEAN matchTable(StatusType* status, const char* relationName) = 0;
		virtual IApplierTransaction* getTransaction(StatusType* status, ISC_INT64 number) = 0;
		virtual void cleanupTransaction(StatusType* status, ISC_INT64 number) = 0;
		virtual void cleanupTransactions(StatusType* status) = 0;
		virtual void log(unsigned level, const char* message) = 0;
	};

	template <typename Name, typename StatusType, typename Base>
	class IStreamLoggerBaseImpl : public Base
	{
	public:
		typedef IStreamLogger Declaration;

		IStreamLoggerBaseImpl(DoNotInherit = DoNotInherit())
		{
			static struct VTableImpl : Base::VTable
			{
				VTableImpl()
				{
					this->version = Base::VERSION;
					this->addRef = &Name::cloopaddRefDispatcher;
					this->release = &Name::cloopreleaseDispatcher;
					this->getLevel = &Name::cloopgetLevelDispatcher;
					this->log = &Name::clooplogDispatcher;
					this->trace = &Name::clooptraceDispatcher;
					this->debug = &Name::cloopdebugDispatcher;
					this->info = &Name::cloopinfoDispatcher;
					this->warning = &Name::cloopwarningDispatcher;
					this->error = &Name::clooperrorDispatcher;
					this->critical = &Name::cloopcriticalDispatcher;
				}
			} vTable;

			this->cloopVTable = &vTable;
		}

		static unsigned CLOOP_CARG cloopgetLevelDispatcher(IStreamLogger* self) throw()
		{
			try
			{
				return static_cast<Name*>(self)->Name::getLevel();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<unsigned>(0);
			}
		}

		static void CLOOP_CARG clooplogDispatcher(IStreamLogger* self, unsigned level, const char* message) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::log(level, message);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static void CLOOP_CARG clooptraceDispatcher(IStreamLogger* self, const char* message) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::trace(message);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static void CLOOP_CARG cloopdebugDispatcher(IStreamLogger* self, const char* message) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::debug(message);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static void CLOOP_CARG cloopinfoDispatcher(IStreamLogger* self, const char* message) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::info(message);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static void CLOOP_CARG cloopwarningDispatcher(IStreamLogger* self, const char* message) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::warning(message);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static void CLOOP_CARG clooperrorDispatcher(IStreamLogger* self, const char* message) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::error(message);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static void CLOOP_CARG cloopcriticalDispatcher(IStreamLogger* self, const char* message) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::critical(message);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static void CLOOP_CARG cloopaddRefDispatcher(IReferenceCounted* self) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::addRef();
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static int CLOOP_CARG cloopreleaseDispatcher(IReferenceCounted* self) throw()
		{
			try
			{
				return static_cast<Name*>(self)->Name::release();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<int>(0);
			}
		}
	};

	template <typename Name, typename StatusType, typename Base = IReferenceCountedImpl<Name, StatusType, Inherit<IVersionedImpl<Name, StatusType, Inherit<IStreamLogger> > > > >
	class IStreamLoggerImpl : public IStreamLoggerBaseImpl<Name, StatusType, Base>
	{
	protected:
		IStreamLoggerImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IStreamLoggerImpl()
		{
		}

		virtual unsigned getLevel() = 0;
		virtual void log(unsigned level, const char* message) = 0;
		virtual void trace(const char* message) = 0;
		virtual void debug(const char* message) = 0;
		virtual void info(const char* message) = 0;
		virtual void warning(const char* message) = 0;
		virtual void error(const char* message) = 0;
		virtual void critical(const char* message) = 0;
	};

	template <typename Name, typename StatusType, typename Base>
	class IReplicateApplierFactoryBaseImpl : public Base
	{
	public:
		typedef IReplicateApplierFactory Declaration;

		IReplicateApplierFactoryBaseImpl(DoNotInherit = DoNotInherit())
		{
			static struct VTableImpl : Base::VTable
			{
				VTableImpl()
				{
					this->version = Base::VERSION;
					this->createPlugin = &Name::cloopcreatePluginDispatcher;
				}
			} vTable;

			this->cloopVTable = &vTable;
		}

		static IReplicateApplierPlugin* CLOOP_CARG cloopcreatePluginDispatcher(IReplicateApplierFactory* self, IStatus* status, IConfig* config, IStreamLogger* logger) throw()
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::createPlugin(&status2, config, logger);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<IReplicateApplierPlugin*>(0);
			}
		}
	};

	template <typename Name, typename StatusType, typename Base = IVersionedImpl<Name, StatusType, Inherit<IReplicateApplierFactory> > >
	class IReplicateApplierFactoryImpl : public IReplicateApplierFactoryBaseImpl<Name, StatusType, Base>
	{
	protected:
		IReplicateApplierFactoryImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IReplicateApplierFactoryImpl()
		{
		}

		virtual IReplicateApplierPlugin* createPlugin(StatusType* status, IConfig* config, IStreamLogger* logger) = 0;
	};

	template <typename Name, typename StatusType, typename Base>
	class IStreamPluginManagerBaseImpl : public Base
	{
	public:
		typedef IStreamPluginManager Declaration;

		IStreamPluginManagerBaseImpl(DoNotInherit = DoNotInherit())
		{
			static struct VTableImpl : Base::VTable
			{
				VTableImpl()
				{
					this->version = Base::VERSION;
					this->dispose = &Name::cloopdisposeDispatcher;
					this->registerPluginFactory = &Name::cloopregisterPluginFactoryDispatcher;
					this->getPlugin = &Name::cloopgetPluginDispatcher;
				}
			} vTable;

			this->cloopVTable = &vTable;
		}

		static void CLOOP_CARG cloopregisterPluginFactoryDispatcher(IStreamPluginManager* self, const char* pluginName, IReplicateApplierFactory* factory) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::registerPluginFactory(pluginName, factory);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static IReplicateApplierPlugin* CLOOP_CARG cloopgetPluginDispatcher(IStreamPluginManager* self, IStatus* status, const char* pluginName, IConfig* config, IStreamLogger* logger) throw()
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::getPlugin(&status2, pluginName, config, logger);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<IReplicateApplierPlugin*>(0);
			}
		}

		static void CLOOP_CARG cloopdisposeDispatcher(IDisposable* self) throw()
		{
			try
			{
				static_cast<Name*>(self)->Name::dispose();
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}
	};

	template <typename Name, typename StatusType, typename Base = IDisposableImpl<Name, StatusType, Inherit<IVersionedImpl<Name, StatusType, Inherit<IStreamPluginManager> > > > >
	class IStreamPluginManagerImpl : public IStreamPluginManagerBaseImpl<Name, StatusType, Base>
	{
	protected:
		IStreamPluginManagerImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IStreamPluginManagerImpl()
		{
		}

		virtual void registerPluginFactory(const char* pluginName, IReplicateApplierFactory* factory) = 0;
		virtual IReplicateApplierPlugin* getPlugin(StatusType* status, const char* pluginName, IConfig* config, IStreamLogger* logger) = 0;
	};
};

#endif
