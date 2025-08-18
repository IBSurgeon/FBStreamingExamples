#pragma once
#ifndef FB_STREAMING_H
#define FB_STREAMING_H

#ifdef _WINDOWS
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include "firebird/Interface.h"

#ifdef _WINDOWS
#pragma warning(pop)
#endif

#ifndef CLOOP_NOEXCEPT
#if __cplusplus >= 201103L
#define CLOOP_NOEXCEPT noexcept
#else
#define CLOOP_NOEXCEPT throw()
#endif
#endif

#ifndef CLOOP_CONSTEXPR
#if __cplusplus >= 201103L
#define CLOOP_CONSTEXPR constexpr
#else
#define CLOOP_CONSTEXPR const
#endif
#endif

namespace Firebird {

	class IStreamedField;
	class IStreamedRecord;
	class IStreamedTransaction;
	class IStreamPlugin;
	class IStreamLogger;
	class IStringConverter;
	class IStringEncodeUtils;
	class IStreamPluginFactory;
	class IStreamPluginManager;

	struct SegmentHeaderInfo {
		char name[256] = {0};
		char guid[39] = {0};
        ISC_UINT64 ts_ms = 0;
		ISC_UINT64 sequence = 0;
		ISC_UINT64 length = 0;
		ISC_USHORT version = 0;
		ISC_USHORT state = 0;      
	};

#define FIREBIRD_ISTREAMED_FIELD_VERSION 2u

	class IStreamedField : public IVersioned
	{
	public:
		struct VTable : public IVersioned::VTable
		{
			const char* (CLOOP_CARG* getName)(IStreamedField* self) CLOOP_NOEXCEPT;
			unsigned (CLOOP_CARG* getType)(IStreamedField* self) CLOOP_NOEXCEPT;
			int (CLOOP_CARG* getSubType)(IStreamedField* self) CLOOP_NOEXCEPT;
			int (CLOOP_CARG* getScale)(IStreamedField* self) CLOOP_NOEXCEPT;
			unsigned (CLOOP_CARG* getLength)(IStreamedField* self) CLOOP_NOEXCEPT;
			unsigned (CLOOP_CARG* getCharSet)(IStreamedField* self) CLOOP_NOEXCEPT;
			const void* (CLOOP_CARG* getData)(IStreamedField* self) CLOOP_NOEXCEPT;
			FB_BOOLEAN(CLOOP_CARG* isKey)(IStreamedField* self) CLOOP_NOEXCEPT;
			unsigned (CLOOP_CARG* keyPosition)(IStreamedField* self) CLOOP_NOEXCEPT;
		};

	protected:
		IStreamedField(DoNotInherit)
			: IVersioned(DoNotInherit())
		{
		}

		~IStreamedField()
		{
		}

	public:
		static CLOOP_CONSTEXPR unsigned VERSION = FIREBIRD_ISTREAMED_FIELD_VERSION;

		const char* getName()
		{
			const char* ret = static_cast<VTable*>(this->cloopVTable)->getName(this);
			return ret;
		}

		unsigned getType()
		{
			unsigned ret = static_cast<VTable*>(this->cloopVTable)->getType(this);
			return ret;
		}

		int getSubType()
		{
			int ret = static_cast<VTable*>(this->cloopVTable)->getSubType(this);
			return ret;
		}

		int getScale()
		{
			int ret = static_cast<VTable*>(this->cloopVTable)->getScale(this);
			return ret;
		}

		unsigned getLength()
		{
			unsigned ret = static_cast<VTable*>(this->cloopVTable)->getLength(this);
			return ret;
		}

		unsigned getCharSet()
		{
			unsigned ret = static_cast<VTable*>(this->cloopVTable)->getCharSet(this);
			return ret;
		}

		const void* getData()
		{
			const void* ret = static_cast<VTable*>(this->cloopVTable)->getData(this);
			return ret;
		}

		FB_BOOLEAN isKey()
		{
			FB_BOOLEAN ret = static_cast<VTable*>(this->cloopVTable)->isKey(this);
			return ret;
		}

		unsigned keyPosition()
		{
			unsigned ret = static_cast<VTable*>(this->cloopVTable)->keyPosition(this);
			return ret;
		}
	};

#define FIREBIRD_ISTREAMED_RECORD_VERSION 2u

	class IStreamedRecord : public IVersioned
	{
	public:
		struct VTable : public IVersioned::VTable
		{
			unsigned (CLOOP_CARG* getCount)(IStreamedRecord* self) CLOOP_NOEXCEPT;
			IStreamedField* (CLOOP_CARG* getField)(IStreamedRecord* self, unsigned index) CLOOP_NOEXCEPT;
			unsigned (CLOOP_CARG* getRawLength)(IStreamedRecord* self) CLOOP_NOEXCEPT;
			const unsigned char* (CLOOP_CARG* getRawData)(IStreamedRecord* self) CLOOP_NOEXCEPT;
		};

	protected:
		IStreamedRecord(DoNotInherit)
			: IVersioned(DoNotInherit())
		{
		}

		~IStreamedRecord()
		{
		}

	public:
		static CLOOP_CONSTEXPR unsigned VERSION = FIREBIRD_ISTREAMED_RECORD_VERSION;

		unsigned getCount()
		{
			unsigned ret = static_cast<VTable*>(this->cloopVTable)->getCount(this);
			return ret;
		}

		IStreamedField* getField(unsigned index)
		{
			IStreamedField* ret = static_cast<VTable*>(this->cloopVTable)->getField(this, index);
			return ret;
		}

		unsigned getRawLength()
		{
			unsigned ret = static_cast<VTable*>(this->cloopVTable)->getRawLength(this);
			return ret;
		}

		const unsigned char* getRawData()
		{
			const unsigned char* ret = static_cast<VTable*>(this->cloopVTable)->getRawData(this);
			return ret;
		}
	};

#define FIREBIRD_ISTREAMED_TRANSACTION_VERSION 3u

	class IStreamedTransaction : public IDisposable
	{
	public:
		struct VTable : public IDisposable::VTable
		{
			void (CLOOP_CARG* prepare)(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* commit)(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* rollback)(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* startSavepoint)(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* releaseSavepoint)(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* rollbackSavepoint)(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* insertRecord)(IStreamedTransaction* self, IStatus* status, const char* name, IStreamedRecord* record) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* updateRecord)(IStreamedTransaction* self, IStatus* status, const char* name, IStreamedRecord* orgRecord, IStreamedRecord* newRecord) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* deleteRecord)(IStreamedTransaction* self, IStatus* status, const char* name, IStreamedRecord* record) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* executeSql)(IStreamedTransaction* self, IStatus* status, const char* sql) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* executeSqlIntl)(IStreamedTransaction* self, IStatus* status, unsigned charset, const char* sql) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* storeBlob)(IStreamedTransaction* self, IStatus* status, ISC_QUAD* blob_id, ISC_INT64 length, const unsigned char* data) CLOOP_NOEXCEPT;
		};

	protected:
		IStreamedTransaction(DoNotInherit)
			: IDisposable(DoNotInherit())
		{
		}

		~IStreamedTransaction()
		{
		}

	public:
		static CLOOP_CONSTEXPR unsigned VERSION = FIREBIRD_ISTREAMED_TRANSACTION_VERSION;

		template <typename StatusType> void prepare(StatusType* status)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->prepare(this, status);
			StatusType::checkException(status);
		}

		template <typename StatusType> void commit(StatusType* status)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->commit(this, status);
			StatusType::checkException(status);
		}

		template <typename StatusType> void rollback(StatusType* status)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->rollback(this, status);
			StatusType::checkException(status);
		}

		template <typename StatusType> void startSavepoint(StatusType* status)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->startSavepoint(this, status);
			StatusType::checkException(status);
		}

		template <typename StatusType> void releaseSavepoint(StatusType* status)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->releaseSavepoint(this, status);
			StatusType::checkException(status);
		}

		template <typename StatusType> void rollbackSavepoint(StatusType* status)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->rollbackSavepoint(this, status);
			StatusType::checkException(status);
		}

		template <typename StatusType> void insertRecord(StatusType* status, const char* name, IStreamedRecord* record)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->insertRecord(this, status, name, record);
			StatusType::checkException(status);
		}

		template <typename StatusType> void updateRecord(StatusType* status, const char* name, IStreamedRecord* orgRecord, IStreamedRecord* newRecord)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->updateRecord(this, status, name, orgRecord, newRecord);
			StatusType::checkException(status);
		}

		template <typename StatusType> void deleteRecord(StatusType* status, const char* name, IStreamedRecord* record)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->deleteRecord(this, status, name, record);
			StatusType::checkException(status);
		}

		template <typename StatusType> void executeSql(StatusType* status, const char* sql)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->executeSql(this, status, sql);
			StatusType::checkException(status);
		}

		template <typename StatusType> void executeSqlIntl(StatusType* status, unsigned charset, const char* sql)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->executeSqlIntl(this, status, charset, sql);
			StatusType::checkException(status);
		}

		template <typename StatusType> void storeBlob(StatusType* status, ISC_QUAD* blob_id, ISC_INT64 length, const unsigned char* data)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->storeBlob(this, status, blob_id, length, data);
			StatusType::checkException(status);
		}
	};

#define FIREBIRD_ISTREAM_PLUGIN_VERSION 4u

	class IStreamPlugin : public IPluginBase
	{
	public:
		struct VTable : public IPluginBase::VTable
		{
			FB_BOOLEAN(CLOOP_CARG* init)(IStreamPlugin* self, IStatus* status, IAttachment* attachment) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* finish)(IStreamPlugin* self, IStatus* status) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* startSegment)(IStreamPlugin* self, IStatus* status, SegmentHeaderInfo* segmentHeader) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* finishSegment)(IStreamPlugin* self, IStatus* status) CLOOP_NOEXCEPT;
			void (CLOOP_CARG *startBlock)(IStreamPlugin* self, IStatus* status, ISC_UINT64 blockOffset, unsigned blockLength) CLOOP_NOEXCEPT;
			void (CLOOP_CARG *setSegmentOffset)(IStreamPlugin* self, ISC_UINT64 offset) CLOOP_NOEXCEPT;
			IStreamedTransaction* (CLOOP_CARG* startTransaction)(IStreamPlugin* self, IStatus* status, ISC_INT64 number) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* setSequence)(IStreamPlugin* self, IStatus* status, const char* name, ISC_INT64 value) CLOOP_NOEXCEPT;
			FB_BOOLEAN(CLOOP_CARG* matchTable)(IStreamPlugin* self, IStatus* status, const char* relationName) CLOOP_NOEXCEPT;
			IStreamedTransaction* (CLOOP_CARG* getTransaction)(IStreamPlugin* self, IStatus* status, ISC_INT64 number) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* cleanupTransaction)(IStreamPlugin* self, IStatus* status, ISC_INT64 number) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* cleanupTransactions)(IStreamPlugin* self, IStatus* status) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* log)(IStreamPlugin* self, unsigned level, const char* message) CLOOP_NOEXCEPT;
		};

	protected:
		IStreamPlugin(DoNotInherit)
			: IPluginBase(DoNotInherit())
		{
		}

		~IStreamPlugin()
		{
		}

	public:
		static CLOOP_CONSTEXPR unsigned VERSION = FIREBIRD_ISTREAM_PLUGIN_VERSION;

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

		template <typename StatusType> void startBlock(StatusType* status, ISC_UINT64 blockOffset, unsigned blockLength)
		{
			StatusType::clearException(status);
			static_cast<VTable*>(this->cloopVTable)->startBlock(this, status, blockOffset, blockLength);
			StatusType::checkException(status);
		}

		void setSegmentOffset(ISC_UINT64 offset)
		{
			static_cast<VTable*>(this->cloopVTable)->setSegmentOffset(this, offset);
		}

		template <typename StatusType> IStreamedTransaction* startTransaction(StatusType* status, ISC_INT64 number)
		{
			StatusType::clearException(status);
			IStreamedTransaction* ret = static_cast<VTable*>(this->cloopVTable)->startTransaction(this, status, number);
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

		template <typename StatusType> IStreamedTransaction* getTransaction(StatusType* status, ISC_INT64 number)
		{
			StatusType::clearException(status);
			IStreamedTransaction* ret = static_cast<VTable*>(this->cloopVTable)->getTransaction(this, status, number);
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

#define FIREBIRD_ISTREAM_LOGGER_VERSION 3u

	class IStreamLogger : public IDisposable
	{
	public:
		struct VTable : public IDisposable::VTable
		{
			unsigned (CLOOP_CARG* getLevel)(IStreamLogger* self) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* setLevel)(IStreamLogger* self, unsigned logLevel) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* log)(IStreamLogger* self, unsigned level, const char* message) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* trace)(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* debug)(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* info)(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* warning)(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* error)(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* critical)(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT;
			void (CLOOP_CARG* flush)(IStreamLogger* self) CLOOP_NOEXCEPT;
		};

	protected:
		IStreamLogger(DoNotInherit)
			: IDisposable(DoNotInherit())
		{
		}

		~IStreamLogger()
		{
		}

	public:
		static CLOOP_CONSTEXPR unsigned VERSION = FIREBIRD_ISTREAM_LOGGER_VERSION;

		static CLOOP_CONSTEXPR unsigned LEVEL_TRACE = 0;
		static CLOOP_CONSTEXPR unsigned LEVEL_DEBUG = 1;
		static CLOOP_CONSTEXPR unsigned LEVEL_INFO = 2;
		static CLOOP_CONSTEXPR unsigned LEVEL_WARN = 3;
		static CLOOP_CONSTEXPR unsigned LEVEL_ERROR = 4;
		static CLOOP_CONSTEXPR unsigned LEVEL_CRITICAL = 5;
		static CLOOP_CONSTEXPR unsigned LEVEL_OFF = 6;

		unsigned getLevel()
		{
			unsigned ret = static_cast<VTable*>(this->cloopVTable)->getLevel(this);
			return ret;
		}

		void setLevel(unsigned logLevel)
		{
			static_cast<VTable*>(this->cloopVTable)->setLevel(this, logLevel);
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

		void flush()
		{
			static_cast<VTable*>(this->cloopVTable)->flush(this);
		}
	};

#define FIREBIRD_ISTRING_CONVERTER_VERSION 3u

	class IStringConverter : public IReferenceCounted
	{
	public:
		struct VTable : public IReferenceCounted::VTable
		{
			int (CLOOP_CARG* getMaxCharSize)(IStringConverter* self) CLOOP_NOEXCEPT;
			int (CLOOP_CARG* getMinCharSize)(IStringConverter* self) CLOOP_NOEXCEPT;
			unsigned (CLOOP_CARG* getCharsetId)(IStringConverter* self) CLOOP_NOEXCEPT;
			const char* (CLOOP_CARG* getCharsetName)(IStringConverter* self) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* toUtf8)(IStringConverter* self, IStatus* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* fromUtf8)(IStringConverter* self, IStatus* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* toUtf16)(IStringConverter* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* fromUtf16)(IStringConverter* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* toUtf32)(IStringConverter* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* fromUtf32)(IStringConverter* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* toWCS)(IStringConverter* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* fromWCS)(IStringConverter* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
		};

	protected:
		IStringConverter(DoNotInherit)
			: IReferenceCounted(DoNotInherit())
		{
		}

		~IStringConverter()
		{
		}

	public:
		static CLOOP_CONSTEXPR unsigned VERSION = FIREBIRD_ISTRING_CONVERTER_VERSION;

		int getMaxCharSize()
		{
			int ret = static_cast<VTable*>(this->cloopVTable)->getMaxCharSize(this);
			return ret;
		}

		int getMinCharSize()
		{
			int ret = static_cast<VTable*>(this->cloopVTable)->getMinCharSize(this);
			return ret;
		}

		unsigned getCharsetId()
		{
			unsigned ret = static_cast<VTable*>(this->cloopVTable)->getCharsetId(this);
			return ret;
		}

		const char* getCharsetName()
		{
			const char* ret = static_cast<VTable*>(this->cloopVTable)->getCharsetName(this);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 toUtf8(StatusType* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->toUtf8(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 fromUtf8(StatusType* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->fromUtf8(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 toUtf16(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->toUtf16(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 fromUtf16(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->fromUtf16(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 toUtf32(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->toUtf32(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 fromUtf32(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->fromUtf32(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 toWCS(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->toWCS(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 fromWCS(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->fromWCS(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}
	};

#define FIREBIRD_ISTRING_ENCODE_UTILS_VERSION 3u

	class IStringEncodeUtils : public IDisposable
	{
	public:
		struct VTable : public IDisposable::VTable
		{
			IStringConverter* (CLOOP_CARG* getConverterById)(IStringEncodeUtils* self, IStatus* status, unsigned charsetId) CLOOP_NOEXCEPT;
			IStringConverter* (CLOOP_CARG* getConverterByName)(IStringEncodeUtils* self, IStatus* status, const char* charsetName) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertCharset)(IStringEncodeUtils* self, IStatus* status, IStringConverter* srcConveter, IStringConverter* dstConveter, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertUtf8ToWCS)(IStringEncodeUtils* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertUtf8ToUtf16)(IStringEncodeUtils* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertUtf8ToUtf32)(IStringEncodeUtils* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertUtf16ToWCS)(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertUtf16ToUtf8)(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertUtf16ToUtf32)(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertUtf32ToWCS)(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertUtf32ToUtf8)(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertUtf32ToUtf16)(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertWCSToUtf8)(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertWCSToUtf16)(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
			ISC_UINT64(CLOOP_CARG* convertWCSToUtf32)(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT;
		};

	protected:
		IStringEncodeUtils(DoNotInherit)
			: IDisposable(DoNotInherit())
		{
		}

		~IStringEncodeUtils()
		{
		}

	public:
		static CLOOP_CONSTEXPR unsigned VERSION = FIREBIRD_ISTRING_ENCODE_UTILS_VERSION;

		template <typename StatusType> IStringConverter* getConverterById(StatusType* status, unsigned charsetId)
		{
			StatusType::clearException(status);
			IStringConverter* ret = static_cast<VTable*>(this->cloopVTable)->getConverterById(this, status, charsetId);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> IStringConverter* getConverterByName(StatusType* status, const char* charsetName)
		{
			StatusType::clearException(status);
			IStringConverter* ret = static_cast<VTable*>(this->cloopVTable)->getConverterByName(this, status, charsetName);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertCharset(StatusType* status, IStringConverter* srcConveter, IStringConverter* dstConveter, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertCharset(this, status, srcConveter, dstConveter, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertUtf8ToWCS(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertUtf8ToWCS(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertUtf8ToUtf16(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertUtf8ToUtf16(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertUtf8ToUtf32(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertUtf8ToUtf32(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertUtf16ToWCS(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertUtf16ToWCS(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertUtf16ToUtf8(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertUtf16ToUtf8(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertUtf16ToUtf32(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertUtf16ToUtf32(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertUtf32ToWCS(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertUtf32ToWCS(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertUtf32ToUtf8(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertUtf32ToUtf8(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertUtf32ToUtf16(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertUtf32ToUtf16(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertWCSToUtf8(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertWCSToUtf8(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertWCSToUtf16(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertWCSToUtf16(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}

		template <typename StatusType> ISC_UINT64 convertWCSToUtf32(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
		{
			StatusType::clearException(status);
			ISC_UINT64 ret = static_cast<VTable*>(this->cloopVTable)->convertWCSToUtf32(this, status, src, srcSize, destBuffer, destBufferSize);
			StatusType::checkException(status);
			return ret;
		}
	};

#define FIREBIRD_ISTREAM_PLUGIN_FACTORY_VERSION 2u

	class IStreamPluginFactory : public IVersioned
	{
	public:
		struct VTable : public IVersioned::VTable
		{
			IStreamPlugin* (CLOOP_CARG* createPlugin)(IStreamPluginFactory* self, IStatus* status, IConfig* config, IStringEncodeUtils* encodeUtils, IStreamLogger* logger) CLOOP_NOEXCEPT;
		};

	protected:
		IStreamPluginFactory(DoNotInherit)
			: IVersioned(DoNotInherit())
		{
		}

		~IStreamPluginFactory()
		{
		}

	public:
		static CLOOP_CONSTEXPR unsigned VERSION = FIREBIRD_ISTREAM_PLUGIN_FACTORY_VERSION;

		template <typename StatusType> IStreamPlugin* createPlugin(StatusType* status, IConfig* config, IStringEncodeUtils* encodeUtils, IStreamLogger* logger)
		{
			StatusType::clearException(status);
			IStreamPlugin* ret = static_cast<VTable*>(this->cloopVTable)->createPlugin(this, status, config, encodeUtils, logger);
			StatusType::checkException(status);
			return ret;
		}
	};

#define FIREBIRD_ISTREAM_PLUGIN_MANAGER_VERSION 3u

	class IStreamPluginManager : public IDisposable
	{
	public:
		struct VTable : public IDisposable::VTable
		{
			void (CLOOP_CARG* registerPluginFactory)(IStreamPluginManager* self, const char* pluginName, IStreamPluginFactory* factory) CLOOP_NOEXCEPT;
			IStreamPlugin* (CLOOP_CARG* getPlugin)(IStreamPluginManager* self, IStatus* status, const char* pluginName, IConfig* config) CLOOP_NOEXCEPT;
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
		static CLOOP_CONSTEXPR unsigned VERSION = FIREBIRD_ISTREAM_PLUGIN_MANAGER_VERSION;

		void registerPluginFactory(const char* pluginName, IStreamPluginFactory* factory)
		{
			static_cast<VTable*>(this->cloopVTable)->registerPluginFactory(this, pluginName, factory);
		}

		template <typename StatusType> IStreamPlugin* getPlugin(StatusType* status, const char* pluginName, IConfig* config)
		{
			StatusType::clearException(status);
			IStreamPlugin* ret = static_cast<VTable*>(this->cloopVTable)->getPlugin(this, status, pluginName, config);
			StatusType::checkException(status);
			return ret;
		}
	};

	template <typename Name, typename StatusType, typename Base>
	class IStreamedFieldBaseImpl : public Base
	{
	public:
		typedef IStreamedField Declaration;

		IStreamedFieldBaseImpl(DoNotInherit = DoNotInherit())
		{
			static struct VTableImpl : Base::VTable
			{
				VTableImpl()
				{
					this->version = Base::VERSION;
					this->getName = &Name::cloopgetNameDispatcher;
					this->getType = &Name::cloopgetTypeDispatcher;
					this->getSubType = &Name::cloopgetSubTypeDispatcher;
					this->getScale = &Name::cloopgetScaleDispatcher;
					this->getLength = &Name::cloopgetLengthDispatcher;
					this->getCharSet = &Name::cloopgetCharSetDispatcher;
					this->getData = &Name::cloopgetDataDispatcher;
					this->isKey = &Name::cloopisKeyDispatcher;
					this->keyPosition = &Name::cloopkeyPositionDispatcher;
				}
			} vTable;

			this->cloopVTable = &vTable;
		}

		static const char* CLOOP_CARG cloopgetNameDispatcher(IStreamedField* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getName();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<const char*>(0);
			}
		}

		static unsigned CLOOP_CARG cloopgetTypeDispatcher(IStreamedField* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getType();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<unsigned>(0);
			}
		}

		static int CLOOP_CARG cloopgetSubTypeDispatcher(IStreamedField* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getSubType();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<int>(0);
			}
		}

		static int CLOOP_CARG cloopgetScaleDispatcher(IStreamedField* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getScale();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<int>(0);
			}
		}

		static unsigned CLOOP_CARG cloopgetLengthDispatcher(IStreamedField* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getLength();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<unsigned>(0);
			}
		}

		static unsigned CLOOP_CARG cloopgetCharSetDispatcher(IStreamedField* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getCharSet();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<unsigned>(0);
			}
		}

		static const void* CLOOP_CARG cloopgetDataDispatcher(IStreamedField* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getData();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<const void*>(0);
			}
		}

		static FB_BOOLEAN CLOOP_CARG cloopisKeyDispatcher(IStreamedField* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::isKey();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<FB_BOOLEAN>(0);
			}
		}

		static unsigned CLOOP_CARG cloopkeyPositionDispatcher(IStreamedField* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::keyPosition();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<unsigned>(0);
			}
		}
	};

	template <typename Name, typename StatusType, typename Base = IVersionedImpl<Name, StatusType, Inherit<IStreamedField> > >
	class IStreamedFieldImpl : public IStreamedFieldBaseImpl<Name, StatusType, Base>
	{
	protected:
		IStreamedFieldImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IStreamedFieldImpl()
		{
		}

		virtual const char* getName() = 0;
		virtual unsigned getType() = 0;
		virtual int getSubType() = 0;
		virtual int getScale() = 0;
		virtual unsigned getLength() = 0;
		virtual unsigned getCharSet() = 0;
		virtual const void* getData() = 0;
		virtual FB_BOOLEAN isKey() = 0;
		virtual unsigned keyPosition() = 0;
	};

	template <typename Name, typename StatusType, typename Base>
	class IStreamedRecordBaseImpl : public Base
	{
	public:
		typedef IStreamedRecord Declaration;

		IStreamedRecordBaseImpl(DoNotInherit = DoNotInherit())
		{
			static struct VTableImpl : Base::VTable
			{
				VTableImpl()
				{
					this->version = Base::VERSION;
					this->getCount = &Name::cloopgetCountDispatcher;
					this->getField = &Name::cloopgetFieldDispatcher;
					this->getRawLength = &Name::cloopgetRawLengthDispatcher;
					this->getRawData = &Name::cloopgetRawDataDispatcher;
				}
			} vTable;

			this->cloopVTable = &vTable;
		}

		static unsigned CLOOP_CARG cloopgetCountDispatcher(IStreamedRecord* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getCount();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<unsigned>(0);
			}
		}

		static IStreamedField* CLOOP_CARG cloopgetFieldDispatcher(IStreamedRecord* self, unsigned index) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getField(index);
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<IStreamedField*>(0);
			}
		}

		static unsigned CLOOP_CARG cloopgetRawLengthDispatcher(IStreamedRecord* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getRawLength();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<unsigned>(0);
			}
		}

		static const unsigned char* CLOOP_CARG cloopgetRawDataDispatcher(IStreamedRecord* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getRawData();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<const unsigned char*>(0);
			}
		}
	};

	template <typename Name, typename StatusType, typename Base = IVersionedImpl<Name, StatusType, Inherit<IStreamedRecord> > >
	class IStreamedRecordImpl : public IStreamedRecordBaseImpl<Name, StatusType, Base>
	{
	protected:
		IStreamedRecordImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IStreamedRecordImpl()
		{
		}

		virtual unsigned getCount() = 0;
		virtual IStreamedField* getField(unsigned index) = 0;
		virtual unsigned getRawLength() = 0;
		virtual const unsigned char* getRawData() = 0;
	};

	template <typename Name, typename StatusType, typename Base>
	class IStreamedTransactionBaseImpl : public Base
	{
	public:
		typedef IStreamedTransaction Declaration;

		IStreamedTransactionBaseImpl(DoNotInherit = DoNotInherit())
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

		static void CLOOP_CARG cloopprepareDispatcher(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopcommitDispatcher(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG clooprollbackDispatcher(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopstartSavepointDispatcher(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopreleaseSavepointDispatcher(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG clooprollbackSavepointDispatcher(IStreamedTransaction* self, IStatus* status) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopinsertRecordDispatcher(IStreamedTransaction* self, IStatus* status, const char* name, IStreamedRecord* record) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopupdateRecordDispatcher(IStreamedTransaction* self, IStatus* status, const char* name, IStreamedRecord* orgRecord, IStreamedRecord* newRecord) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopdeleteRecordDispatcher(IStreamedTransaction* self, IStatus* status, const char* name, IStreamedRecord* record) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopexecuteSqlDispatcher(IStreamedTransaction* self, IStatus* status, const char* sql) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopexecuteSqlIntlDispatcher(IStreamedTransaction* self, IStatus* status, unsigned charset, const char* sql) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopstoreBlobDispatcher(IStreamedTransaction* self, IStatus* status, ISC_QUAD* blob_id, ISC_INT64 length, const unsigned char* data) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopdisposeDispatcher(IDisposable* self) CLOOP_NOEXCEPT
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

	template <typename Name, typename StatusType, typename Base = IDisposableImpl<Name, StatusType, Inherit<IVersionedImpl<Name, StatusType, Inherit<IStreamedTransaction> > > > >
	class IStreamedTransactionImpl : public IStreamedTransactionBaseImpl<Name, StatusType, Base>
	{
	protected:
		IStreamedTransactionImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IStreamedTransactionImpl()
		{
		}

		virtual void prepare(StatusType* status) = 0;
		virtual void commit(StatusType* status) = 0;
		virtual void rollback(StatusType* status) = 0;
		virtual void startSavepoint(StatusType* status) = 0;
		virtual void releaseSavepoint(StatusType* status) = 0;
		virtual void rollbackSavepoint(StatusType* status) = 0;
		virtual void insertRecord(StatusType* status, const char* name, IStreamedRecord* record) = 0;
		virtual void updateRecord(StatusType* status, const char* name, IStreamedRecord* orgRecord, IStreamedRecord* newRecord) = 0;
		virtual void deleteRecord(StatusType* status, const char* name, IStreamedRecord* record) = 0;
		virtual void executeSql(StatusType* status, const char* sql) = 0;
		virtual void executeSqlIntl(StatusType* status, unsigned charset, const char* sql) = 0;
		virtual void storeBlob(StatusType* status, ISC_QUAD* blob_id, ISC_INT64 length, const unsigned char* data) = 0;
	};

	template <typename Name, typename StatusType, typename Base>
	class IStreamPluginBaseImpl : public Base
	{
	public:
		typedef IStreamPlugin Declaration;

		IStreamPluginBaseImpl(DoNotInherit = DoNotInherit())
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
					this->startBlock = &Name::cloopstartBlockDispatcher;
					this->setSegmentOffset = &Name::cloopsetSegmentOffsetDispatcher;					
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

		static FB_BOOLEAN CLOOP_CARG cloopinitDispatcher(IStreamPlugin* self, IStatus* status, IAttachment* attachment) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopfinishDispatcher(IStreamPlugin* self, IStatus* status) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopstartSegmentDispatcher(IStreamPlugin* self, IStatus* status, SegmentHeaderInfo* segmentHeader) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopfinishSegmentDispatcher(IStreamPlugin* self, IStatus* status) CLOOP_NOEXCEPT
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
		
		static void CLOOP_CARG cloopstartBlockDispatcher(IStreamPlugin* self, IStatus* status, ISC_UINT64 blockOffset, unsigned blockLength) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				static_cast<Name*>(self)->Name::startBlock(&status2, blockOffset, blockLength);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
			}
		}

		static void CLOOP_CARG cloopsetSegmentOffsetDispatcher(IStreamPlugin* self, ISC_UINT64 offset) CLOOP_NOEXCEPT
		{
			try
			{
				static_cast<Name*>(self)->Name::setSegmentOffset(offset);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}		

		static IStreamedTransaction* CLOOP_CARG cloopstartTransactionDispatcher(IStreamPlugin* self, IStatus* status, ISC_INT64 number) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::startTransaction(&status2, number);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<IStreamedTransaction*>(0);
			}
		}

		static void CLOOP_CARG cloopsetSequenceDispatcher(IStreamPlugin* self, IStatus* status, const char* name, ISC_INT64 value) CLOOP_NOEXCEPT
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

		static FB_BOOLEAN CLOOP_CARG cloopmatchTableDispatcher(IStreamPlugin* self, IStatus* status, const char* relationName) CLOOP_NOEXCEPT
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

		static IStreamedTransaction* CLOOP_CARG cloopgetTransactionDispatcher(IStreamPlugin* self, IStatus* status, ISC_INT64 number) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::getTransaction(&status2, number);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<IStreamedTransaction*>(0);
			}
		}

		static void CLOOP_CARG cloopcleanupTransactionDispatcher(IStreamPlugin* self, IStatus* status, ISC_INT64 number) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopcleanupTransactionsDispatcher(IStreamPlugin* self, IStatus* status) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG clooplogDispatcher(IStreamPlugin* self, unsigned level, const char* message) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopsetOwnerDispatcher(IPluginBase* self, IReferenceCounted* r) CLOOP_NOEXCEPT
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

		static IReferenceCounted* CLOOP_CARG cloopgetOwnerDispatcher(IPluginBase* self) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopaddRefDispatcher(IReferenceCounted* self) CLOOP_NOEXCEPT
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

		static int CLOOP_CARG cloopreleaseDispatcher(IReferenceCounted* self) CLOOP_NOEXCEPT
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

	template <typename Name, typename StatusType, typename Base = IPluginBaseImpl<Name, StatusType, Inherit<IReferenceCountedImpl<Name, StatusType, Inherit<IVersionedImpl<Name, StatusType, Inherit<IStreamPlugin> > > > > > >
	class IStreamPluginImpl : public IStreamPluginBaseImpl<Name, StatusType, Base>
	{
	protected:
		IStreamPluginImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IStreamPluginImpl()
		{
		}

		virtual FB_BOOLEAN init(StatusType* status, IAttachment* attachment) = 0;
		virtual void finish(StatusType* status) = 0;
		virtual void startSegment(StatusType* status, SegmentHeaderInfo* segmentHeader) = 0;
		virtual void finishSegment(StatusType* status) = 0;
		virtual void startBlock(StatusType* status, ISC_UINT64 blockOffset, unsigned blockLength) = 0;
		virtual void setSegmentOffset(ISC_UINT64 offset) = 0;
		virtual IStreamedTransaction* startTransaction(StatusType* status, ISC_INT64 number) = 0;
		virtual void setSequence(StatusType* status, const char* name, ISC_INT64 value) = 0;
		virtual FB_BOOLEAN matchTable(StatusType* status, const char* relationName) = 0;
		virtual IStreamedTransaction* getTransaction(StatusType* status, ISC_INT64 number) = 0;
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
					this->dispose = &Name::cloopdisposeDispatcher;
					this->getLevel = &Name::cloopgetLevelDispatcher;
					this->setLevel = &Name::cloopsetLevelDispatcher;
					this->log = &Name::clooplogDispatcher;
					this->trace = &Name::clooptraceDispatcher;
					this->debug = &Name::cloopdebugDispatcher;
					this->info = &Name::cloopinfoDispatcher;
					this->warning = &Name::cloopwarningDispatcher;
					this->error = &Name::clooperrorDispatcher;
					this->critical = &Name::cloopcriticalDispatcher;
					this->flush = &Name::cloopflushDispatcher;
				}
			} vTable;

			this->cloopVTable = &vTable;
		}

		static unsigned CLOOP_CARG cloopgetLevelDispatcher(IStreamLogger* self) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopsetLevelDispatcher(IStreamLogger* self, unsigned logLevel) CLOOP_NOEXCEPT
		{
			try
			{
				static_cast<Name*>(self)->Name::setLevel(logLevel);
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static void CLOOP_CARG clooplogDispatcher(IStreamLogger* self, unsigned level, const char* message) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG clooptraceDispatcher(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopdebugDispatcher(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopinfoDispatcher(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopwarningDispatcher(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG clooperrorDispatcher(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopcriticalDispatcher(IStreamLogger* self, const char* message) CLOOP_NOEXCEPT
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

		static void CLOOP_CARG cloopflushDispatcher(IStreamLogger* self) CLOOP_NOEXCEPT
		{
			try
			{
				static_cast<Name*>(self)->Name::flush();
			}
			catch (...)
			{
				StatusType::catchException(0);
			}
		}

		static void CLOOP_CARG cloopdisposeDispatcher(IDisposable* self) CLOOP_NOEXCEPT
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

	template <typename Name, typename StatusType, typename Base = IDisposableImpl<Name, StatusType, Inherit<IVersionedImpl<Name, StatusType, Inherit<IStreamLogger> > > > >
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
		virtual void setLevel(unsigned logLevel) = 0;
		virtual void log(unsigned level, const char* message) = 0;
		virtual void trace(const char* message) = 0;
		virtual void debug(const char* message) = 0;
		virtual void info(const char* message) = 0;
		virtual void warning(const char* message) = 0;
		virtual void error(const char* message) = 0;
		virtual void critical(const char* message) = 0;
		virtual void flush() = 0;
	};

	template <typename Name, typename StatusType, typename Base>
	class IStringConverterBaseImpl : public Base
	{
	public:
		typedef IStringConverter Declaration;

		IStringConverterBaseImpl(DoNotInherit = DoNotInherit())
		{
			static struct VTableImpl : Base::VTable
			{
				VTableImpl()
				{
					this->version = Base::VERSION;
					this->addRef = &Name::cloopaddRefDispatcher;
					this->release = &Name::cloopreleaseDispatcher;
					this->getMaxCharSize = &Name::cloopgetMaxCharSizeDispatcher;
					this->getMinCharSize = &Name::cloopgetMinCharSizeDispatcher;
					this->getCharsetId = &Name::cloopgetCharsetIdDispatcher;
					this->getCharsetName = &Name::cloopgetCharsetNameDispatcher;
					this->toUtf8 = &Name::clooptoUtf8Dispatcher;
					this->fromUtf8 = &Name::cloopfromUtf8Dispatcher;
					this->toUtf16 = &Name::clooptoUtf16Dispatcher;
					this->fromUtf16 = &Name::cloopfromUtf16Dispatcher;
					this->toUtf32 = &Name::clooptoUtf32Dispatcher;
					this->fromUtf32 = &Name::cloopfromUtf32Dispatcher;
					this->toWCS = &Name::clooptoWCSDispatcher;
					this->fromWCS = &Name::cloopfromWCSDispatcher;
				}
			} vTable;

			this->cloopVTable = &vTable;
		}

		static int CLOOP_CARG cloopgetMaxCharSizeDispatcher(IStringConverter* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getMaxCharSize();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<int>(0);
			}
		}

		static int CLOOP_CARG cloopgetMinCharSizeDispatcher(IStringConverter* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getMinCharSize();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<int>(0);
			}
		}

		static unsigned CLOOP_CARG cloopgetCharsetIdDispatcher(IStringConverter* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getCharsetId();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<unsigned>(0);
			}
		}

		static const char* CLOOP_CARG cloopgetCharsetNameDispatcher(IStringConverter* self) CLOOP_NOEXCEPT
		{
			try
			{
				return static_cast<Name*>(self)->Name::getCharsetName();
			}
			catch (...)
			{
				StatusType::catchException(0);
				return static_cast<const char*>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG clooptoUtf8Dispatcher(IStringConverter* self, IStatus* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::toUtf8(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopfromUtf8Dispatcher(IStringConverter* self, IStatus* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::fromUtf8(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG clooptoUtf16Dispatcher(IStringConverter* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::toUtf16(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopfromUtf16Dispatcher(IStringConverter* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::fromUtf16(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG clooptoUtf32Dispatcher(IStringConverter* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::toUtf32(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopfromUtf32Dispatcher(IStringConverter* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::fromUtf32(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG clooptoWCSDispatcher(IStringConverter* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::toWCS(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopfromWCSDispatcher(IStringConverter* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::fromWCS(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static void CLOOP_CARG cloopaddRefDispatcher(IReferenceCounted* self) CLOOP_NOEXCEPT
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

		static int CLOOP_CARG cloopreleaseDispatcher(IReferenceCounted* self) CLOOP_NOEXCEPT
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

	template <typename Name, typename StatusType, typename Base = IReferenceCountedImpl<Name, StatusType, Inherit<IVersionedImpl<Name, StatusType, Inherit<IStringConverter> > > > >
	class IStringConverterImpl : public IStringConverterBaseImpl<Name, StatusType, Base>
	{
	protected:
		IStringConverterImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IStringConverterImpl()
		{
		}

		virtual int getMaxCharSize() = 0;
		virtual int getMinCharSize() = 0;
		virtual unsigned getCharsetId() = 0;
		virtual const char* getCharsetName() = 0;
		virtual ISC_UINT64 toUtf8(StatusType* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 fromUtf8(StatusType* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 toUtf16(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 fromUtf16(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 toUtf32(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 fromUtf32(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 toWCS(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 fromWCS(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) = 0;
	};

	template <typename Name, typename StatusType, typename Base>
	class IStringEncodeUtilsBaseImpl : public Base
	{
	public:
		typedef IStringEncodeUtils Declaration;

		IStringEncodeUtilsBaseImpl(DoNotInherit = DoNotInherit())
		{
			static struct VTableImpl : Base::VTable
			{
				VTableImpl()
				{
					this->version = Base::VERSION;
					this->dispose = &Name::cloopdisposeDispatcher;
					this->getConverterById = &Name::cloopgetConverterByIdDispatcher;
					this->getConverterByName = &Name::cloopgetConverterByNameDispatcher;
					this->convertCharset = &Name::cloopconvertCharsetDispatcher;
					this->convertUtf8ToWCS = &Name::cloopconvertUtf8ToWCSDispatcher;
					this->convertUtf8ToUtf16 = &Name::cloopconvertUtf8ToUtf16Dispatcher;
					this->convertUtf8ToUtf32 = &Name::cloopconvertUtf8ToUtf32Dispatcher;
					this->convertUtf16ToWCS = &Name::cloopconvertUtf16ToWCSDispatcher;
					this->convertUtf16ToUtf8 = &Name::cloopconvertUtf16ToUtf8Dispatcher;
					this->convertUtf16ToUtf32 = &Name::cloopconvertUtf16ToUtf32Dispatcher;
					this->convertUtf32ToWCS = &Name::cloopconvertUtf32ToWCSDispatcher;
					this->convertUtf32ToUtf8 = &Name::cloopconvertUtf32ToUtf8Dispatcher;
					this->convertUtf32ToUtf16 = &Name::cloopconvertUtf32ToUtf16Dispatcher;
					this->convertWCSToUtf8 = &Name::cloopconvertWCSToUtf8Dispatcher;
					this->convertWCSToUtf16 = &Name::cloopconvertWCSToUtf16Dispatcher;
					this->convertWCSToUtf32 = &Name::cloopconvertWCSToUtf32Dispatcher;
				}
			} vTable;

			this->cloopVTable = &vTable;
		}

		static IStringConverter* CLOOP_CARG cloopgetConverterByIdDispatcher(IStringEncodeUtils* self, IStatus* status, unsigned charsetId) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::getConverterById(&status2, charsetId);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<IStringConverter*>(0);
			}
		}

		static IStringConverter* CLOOP_CARG cloopgetConverterByNameDispatcher(IStringEncodeUtils* self, IStatus* status, const char* charsetName) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::getConverterByName(&status2, charsetName);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<IStringConverter*>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertCharsetDispatcher(IStringEncodeUtils* self, IStatus* status, IStringConverter* srcConveter, IStringConverter* dstConveter, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertCharset(&status2, srcConveter, dstConveter, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertUtf8ToWCSDispatcher(IStringEncodeUtils* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertUtf8ToWCS(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertUtf8ToUtf16Dispatcher(IStringEncodeUtils* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertUtf8ToUtf16(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertUtf8ToUtf32Dispatcher(IStringEncodeUtils* self, IStatus* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertUtf8ToUtf32(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertUtf16ToWCSDispatcher(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertUtf16ToWCS(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertUtf16ToUtf8Dispatcher(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertUtf16ToUtf8(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertUtf16ToUtf32Dispatcher(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertUtf16ToUtf32(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertUtf32ToWCSDispatcher(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertUtf32ToWCS(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertUtf32ToUtf8Dispatcher(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertUtf32ToUtf8(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertUtf32ToUtf16Dispatcher(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertUtf32ToUtf16(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertWCSToUtf8Dispatcher(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertWCSToUtf8(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertWCSToUtf16Dispatcher(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertWCSToUtf16(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static ISC_UINT64 CLOOP_CARG cloopconvertWCSToUtf32Dispatcher(IStringEncodeUtils* self, IStatus* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::convertWCSToUtf32(&status2, src, srcSize, destBuffer, destBufferSize);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<ISC_UINT64>(0);
			}
		}

		static void CLOOP_CARG cloopdisposeDispatcher(IDisposable* self) CLOOP_NOEXCEPT
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

	template <typename Name, typename StatusType, typename Base = IDisposableImpl<Name, StatusType, Inherit<IVersionedImpl<Name, StatusType, Inherit<IStringEncodeUtils> > > > >
	class IStringEncodeUtilsImpl : public IStringEncodeUtilsBaseImpl<Name, StatusType, Base>
	{
	protected:
		IStringEncodeUtilsImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IStringEncodeUtilsImpl()
		{
		}

		virtual IStringConverter* getConverterById(StatusType* status, unsigned charsetId) = 0;
		virtual IStringConverter* getConverterByName(StatusType* status, const char* charsetName) = 0;
		virtual ISC_UINT64 convertCharset(StatusType* status, IStringConverter* srcConveter, IStringConverter* dstConveter, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertUtf8ToWCS(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertUtf8ToUtf16(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertUtf8ToUtf32(StatusType* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertUtf16ToWCS(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertUtf16ToUtf8(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertUtf16ToUtf32(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertUtf32ToWCS(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertUtf32ToUtf8(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertUtf32ToUtf16(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertWCSToUtf8(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertWCSToUtf16(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
		virtual ISC_UINT64 convertWCSToUtf32(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) = 0;
	};

	template <typename Name, typename StatusType, typename Base>
	class IStreamPluginFactoryBaseImpl : public Base
	{
	public:
		typedef IStreamPluginFactory Declaration;

		IStreamPluginFactoryBaseImpl(DoNotInherit = DoNotInherit())
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

		static IStreamPlugin* CLOOP_CARG cloopcreatePluginDispatcher(IStreamPluginFactory* self, IStatus* status, IConfig* config, IStringEncodeUtils* encodeUtils, IStreamLogger* logger) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::createPlugin(&status2, config, encodeUtils, logger);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<IStreamPlugin*>(0);
			}
		}
	};

	template <typename Name, typename StatusType, typename Base = IVersionedImpl<Name, StatusType, Inherit<IStreamPluginFactory> > >
	class IStreamPluginFactoryImpl : public IStreamPluginFactoryBaseImpl<Name, StatusType, Base>
	{
	protected:
		IStreamPluginFactoryImpl(DoNotInherit = DoNotInherit())
		{
		}

	public:
		virtual ~IStreamPluginFactoryImpl()
		{
		}

		virtual IStreamPlugin* createPlugin(StatusType* status, IConfig* config, IStringEncodeUtils* encodeUtils, IStreamLogger* logger) = 0;
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

		static void CLOOP_CARG cloopregisterPluginFactoryDispatcher(IStreamPluginManager* self, const char* pluginName, IStreamPluginFactory* factory) CLOOP_NOEXCEPT
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

		static IStreamPlugin* CLOOP_CARG cloopgetPluginDispatcher(IStreamPluginManager* self, IStatus* status, const char* pluginName, IConfig* config) CLOOP_NOEXCEPT
		{
			StatusType status2(status);

			try
			{
				return static_cast<Name*>(self)->Name::getPlugin(&status2, pluginName, config);
			}
			catch (...)
			{
				StatusType::catchException(&status2);
				return static_cast<IStreamPlugin*>(0);
			}
		}

		static void CLOOP_CARG cloopdisposeDispatcher(IDisposable* self) CLOOP_NOEXCEPT
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

		virtual void registerPluginFactory(const char* pluginName, IStreamPluginFactory* factory) = 0;
		virtual IStreamPlugin* getPlugin(StatusType* status, const char* pluginName, IConfig* config) = 0;
	};

} // namespace Firebird

#endif // FB_STREAMING_H
