# Написание собственного плагина для службы fb_streaming

Вы можете написать собственный плагин публикации событий на основе анализа журналов репликации для службы `fb_streaming`.

Плагин может быть написан на языке C++ или Delphi.

Для создания плагина вам необходимо написать реализацию трёх интерфейсов:

* `IStreamPlugin` - обработчик не транзакционных событий и события старта транзакции;
* `IStreamedTransaction` - обработчик транзакционных событий;
* `IStreamPluginFactory` - фабрика для создания экземпляра плагина с интерфейсом `IStreamPlugin`.

Для упрощения реализации этих интерфейсов существуют специальные шаблонные классы с суффиксом `Impl`: `IStreamPluginImpl`, `IStreamedTransactionImpl` и `IStreamPluginFactoryImpl`. В качестве первого параметра шаблона указывается имя класса реализации, в качестве второго - имя класса обёртки для статуса. В качестве обёрток для статуса доступны два класса: `ThrowStatusWrapper` и `CheckStatusWrapper`.

Класс `ThrowStatusWrapper` немедленно генерирует исключение, если статус вектор содержит ошибку. Класс `CheckStatusWrapper` позволяет проверить наличие ошибки позже, например в вызываемом коде, с помощью метода `isDirty()`.

Допустим мы решили создать классы, которые реализуют данные интерфейсы со следующими названиями:
`MyStreamPlugin`, `MyStreamedTransaction` и `MyStreamPluginFactory`. Тогда определение этих классов должно выглядеть следующим образом:

```cpp
namespace MyPlugin {

    class MyStreamedTransaction;

    class MyStreamPlugin : public Firebird::IStreamPluginImpl<MyStreamPlugin, Firebird::ThrowStatusWrapper>
    {
    public:
        MyStreamPlugin() = delete;
        MyStreamPlugin(Firebird::IMaster* master, Firebird::IConfig* config, 
            Firebird::IStringEncodeUtils* encodeUtils, Firebird::IStreamLogger* logger);
        virtual ~MyStreamPlugin();

        // IReferenceCounted implementation
        void addRef() override;
        int release() override;

        // IPluginBase implementation
        void setOwner(Firebird::IReferenceCounted* o) override;
        Firebird::IReferenceCounted* getOwner() override;

        // IStreamPlugin implementation
        FB_BOOLEAN init(Firebird::ThrowStatusWrapper* status, Firebird::IAttachment* attachment) override;
        void finish(Firebird::ThrowStatusWrapper* status) override;
        void startSegment(Firebird::ThrowStatusWrapper* status, SegmentHeaderInfo* segmentHeader) override;
        void finishSegment(Firebird::ThrowStatusWrapper* status) override;
        void startBlock(Firebird::ThrowStatusWrapper* status, unsigned blockOffset) override;
        void setSegmentOffset(unsigned offset) override;
        Firebird::IStreamedTransaction* startTransaction(Firebird::ThrowStatusWrapper* status, ISC_INT64 number) override;
        void setSequence(Firebird::ThrowStatusWrapper* status, const char* name, ISC_INT64 value) override;
        FB_BOOLEAN matchTable(Firebird::ThrowStatusWrapper* status, const char* relationName) override;
        Firebird::IStreamedTransaction* getTransaction(Firebird::ThrowStatusWrapper* status, ISC_INT64 number) override;
        void cleanupTransaction(Firebird::ThrowStatusWrapper* status, ISC_INT64 number) override;
        void cleanupTransactions(Firebird::ThrowStatusWrapper* status) override;
        void log(unsigned level, const char* message) override;
    private:
        // ...
    };

    class MyStreamedTransaction : public Firebird::IStreamedTransactionImpl<MyStreamedTransaction, Firebird::ThrowStatusWrapper>
    {
    public:
        MyStreamedTransaction() = delete;
        MyStreamedTransaction(MyStreamPlugin* streamPlugin, ISC_INT64 number);
        virtual ~MyStreamedTransaction();

        // IDisposable implementation
        void dispose() override;

        // IStreamedTransaction implementation
        void prepare(Firebird::ThrowStatusWrapper* status) override;
        void commit(Firebird::ThrowStatusWrapper* status) override;
        void rollback(Firebird::ThrowStatusWrapper* status) override;
        void startSavepoint(Firebird::ThrowStatusWrapper* status) override;
        void releaseSavepoint(Firebird::ThrowStatusWrapper* status) override;
        void rollbackSavepoint(Firebird::ThrowStatusWrapper* status) override;
        void insertRecord(Firebird::ThrowStatusWrapper* status, const char* name, Firebird::IStreamedRecord* record) override;
        void updateRecord(Firebird::ThrowStatusWrapper* status, const char* name, Firebird::IStreamedRecord* orgRecord, 
            Firebird::IStreamedRecord* newRecord) override;
        void deleteRecord(Firebird::ThrowStatusWrapper* status, const char* name, Firebird::IStreamedRecord* record) override;
        void executeSql(Firebird::ThrowStatusWrapper* status, const char* sql) override;
        void executeSqlIntl(Firebird::ThrowStatusWrapper* status, unsigned charset, const char* sql) override;
        void storeBlob(Firebird::ThrowStatusWrapper* status, ISC_QUAD* blob_id,
            ISC_INT64 length, const unsigned char* data) override;
    private:
        MyStreamPlugin* m_streamPlugin;
        ISC_INT64 m_number;
        // other fields
    };


    class MyStreamPluginFactory : public Firebird::IStreamPluginFactoryImpl<MyStreamPluginFactory, Firebird::ThrowStatusWrapper>
    {
    private:
        Firebird::IMaster* m_master = nullptr;
    public:
        MyStreamPluginFactory() = delete;
        explicit MyStreamPluginFactory(Firebird::IMaster* master);

    Firebird::IStreamPlugin* createPlugin(Firebird::ThrowStatusWrapper* status, Firebird::IConfig* config,
        Firebird::IStringEncodeUtils* encodeUtils, Firebird::IStreamLogger* logger) override;
    };

    // ...
} // namespace MyPlugin    
```

После того, как вы написали реализацию данных интерфейсов, необходимо зарегистрировать плагин в точке входа. Для плагинов службы `fb_streaming` точкой входа является функция с именем `fb_stream_plugin`.

Пример регистрации плагина приведён в следующем коде:

```cpp

#include "../../include/StreamingInterface.h"

using namespace Firebird;

extern "C" {

    FB_DLL_EXPORT void fb_stream_plugin(IMaster* master, IStreamPluginManager* pm)
    {
        // создание фабрики для регистрации плагина
        auto factory = new MyPlugin::MyStreamPluginFactory(master);
        // регистрируем плагин с именем "my_plugin"
        pm->registerPluginFactory("my_plugin", factory);
    }
} // extern C 
```

Полный пример создания собственного плагина для службы `fb_streaming` с исходным кодом вы можете получить по адресу <https://github.com/IBSurgeon/FBStreamingExamples>

Далее приведено описание интерфейсов, которые потребуются вам при реализации собственных плагинов.

---

## Интерфейс `IStreamPlugin`

Интерфейс `IStreamPlugin` - содержит функции для инициализации и завершения работы плагина, фильтрации таблиц по имени, обработчики начала и завершения обработки сегмента журнала репликации, старта транзакции, установки значения генератора (последовательности) и другие функции.

Интерфейс `IStreamPlugin` является потомком интерфейса `IPluginBase`. Это обозначает, что помимо родных функций интерфейса вы должны реализовать методы: `setOwner` и `getOwner`. Кроме того, вы должны реализовать методы интерфейса подсчёта ссылок `IReferenceCounted`: `addRef` и `release`.

### Функция `init`

```cpp
FB_BOOLEAN init(Status* status, IAttachment* attachment)
```

Функция `init` предназначена для инициализации плагина. В параметр `attachment` передаётся экземпляр соединения с master-базой данных. Это может быть необходимо для чтения метаданных, необходимых для функционирования плагина. Функция возвращает `true` в случае успеха, и `false` в противном случае.

> [!NOTE]
> Функция `init` может вызываться более одного раза, поскольку в случае простоя соединения с базой данных разрывается.

### Функция `finish`

```cpp
void finish(Status* status)
```

Функция `finish` предназначена для очистки и освобождения структур выделенных в процессе инициализации плагина.

### Функция `startSegment`

```cpp
void startSegment(Status* status, SegmentHeaderInfo* segmentHeader)
```

Функция `startSegment` предназначена для обработки события старта обработки сегмента репликации.
В параметр `segmentHeader` передаётся структура с информацией из заголовка сегмента репликации.

Структура `SegmentHeaderInfo` выглядит следующим образом:

```cpp
    struct SegmentHeaderInfo
    {
        char name[256];
        char guid[39];
        ISC_UINT64 sequence;
        ISC_UINT64 length;
        ISC_USHORT version;
        ISC_USHORT state;
    };
```

Описание полей структуры:

* `name` - имя файла сегмента репликации;
* `guid` - GUID мастер-базы данных;
* `sequence` - номер сегмента репликации;
* `length` - длина сегмента репликации в байтах;
* `version` - версия протокола репликации;
* `state` - состояние сегмента репликации. 0 - free, 1 - used, 2 - full, 3 - archive.

### Функция `finishSegment`

```cpp
void finishSegment(Status* status)
```

Функция `finishSegment` предназначена для обработки события завершения обработки сегмента репликации.

### Функция `startBlock`

```cpp
void startBlock(ThrowStatusWrapper* status, unsigned blockOffset) override;
```

Функция `startBlock` предназначена для обработки события начала блока в файле сегмента репликации. Обычно события в файл не по одному, а целыми блоками. Если транзакция достаточно короткая, то начало и конец блока совпадают с началом и завершением транзакции. В ваших плагинах вы скорее всего сделаете этот метод заглушкой. Он используется в приложении `fb_repl_print` для вывода на печать информации о начале блока.

### Функция `setSegmentOffset`

```cpp
void setSegmentOffset(unsigned offset) override;
```

Сохраняет смещение (в байтах) из файла сегмента репликации. В большинстве плагинов данный метод является просто заглушкой. Однако он используется в приложении `fb_repl_print`, чтобы указать смещение для каждого события при печати.

### Функция `startTransaction`

```cpp
IStreamedTransaction* startTransaction(Status* status, ISC_INT64 number)
```

Функция `startTransaction` предназначена для обработки события старта транзакции. В параметр `number` передаётся номер транзакции. Функция должна создавать экземпляр класса реализующего интерфейс `IStreamedTransaction` и возвращать указатель на него.

Список указателей на интерфейс `IStreamedTransaction` должен хранится во внутренних структурах класса реализующего интерфейс `IStreamPlugin`. Это необходимо, поскольку завершение событие завершения транзакции может быть отложено на неопределённое время (транзакция необязательно завершается в том же сегменте в котором была начата).

### Функция `setSequence`

```cpp
void setSequence(Status* status, const char* name, ISC_INT64 value)
```

Функция `setSequence` предназначена для обработки события установки значения генератора (последовательности). В параметр `name` передаётся имя генератора (последовательности), а
в параметр `value` - значение.

### Функция `matchTable`

```cpp
FB_BOOLEAN matchTable(Status* status, const char* relationName)
```

Функция `matchTable` предназначена для фильтрации таблиц по имени. Если функция возвращает `true`, то для таблицы с именем `relationName` будут обрабатываться события `INSERT`, `UPDATE` и `DELETE`, в противном случае события таблицы будут проигнорированы.

### Функция `getTransaction`

```cpp
IStreamedTransaction* getTransaction(Status* status, ISC_INT64 number)
```

Функция `getTransaction` возвращает экземпляр класса реализующего интерфейс `IStreamedTransaction` по номеру транзакции заданному в параметре `number`.

### Функция `cleanupTransaction`

```cpp
void cleanupTransaction(Status* status, ISC_INT64 number)
```

Функция `cleanupTransaction` удаляет экземпляр класса реализующего интерфейс `IStreamedTransaction` из внутреннего списка по номеру транзакции заданному в параметре `number`.

### Функция `cleanupTransactions`

```cpp
void cleanupTransactions(Status* status)
```

Функция `cleanupTransactions` предназначена для обработки события очистки активных (не завершённых) транзакций (экземпляров классов интерфейса `IStreamedTransaction`). Для каждой активной транзакции в списке транзакций вызывается функция `rollback` и транзакция удаляется из внутреннего списка.

### Функция `log`

```cpp
void log(unsigned level, const char* message)
```

Функция `log` предназначена для записи сообщения в журнала работы службы `fb_streaming`. Данную функцию удобно использовать для отладки работы плагина.

В параметр `level` передаётся уровень логирования. Уровни логирования описаны в константах интерфейса `IStreamLogger`:

* `IStreamLogger::LEVEL_TRACE`
* `IStreamLogger::LEVEL_DEBUG`
* `IStreamLogger::LEVEL_INFO`
* `IStreamLogger::LEVEL_WARN`
* `IStreamLogger::LEVEL_ERROR`
* `IStreamLogger::LEVEL_CRITICAL`

В параметр `message` передаётся текст сообщения.

Минимальный уровень логирования сообщений настраивается в конфигурационном файле `fb_streaming.conf` в параметре `logLevel`.

---

## Интерфейс `IStreamedTransaction`

Интерфейс `IStreamedTransaction` - содержит функции регистрации событий в контексте транзакции.

Интерфейс `IStreamedTransaction` является потомком интерфейса `IDisposable`. Это обозначает, что помимо родных функций интерфейса вы должны реализовать метод `dispose`, предназначенный для освобождения выделенных ресурсов и уничтожения экземпляра класса.

### Функция `prepare`

```cpp
void prepare(Status* status)
```

Функция `prepare` предназначена обработки события выполнения первой фазы подтверждения двухфазной транзакции.

### Функция `commit`

```cpp
void commit(Status* status)
```

Функция `commit` предназначена обработки события подтверждения транзакции.

### Функция `rollback`

```cpp
void rollback(Status* status)
```

Функция `rollback` предназначена обработки события отката транзакции.

### Функция `startSavepoint`

```cpp
void startSavepoint(Status* status)
```

Функция `startSavepoint` предназначена обработки события установки точки сохранения.

### Функция `releaseSavepoint`

```cpp
void releaseSavepoint(Status* status)
```

Функция `releaseSavepoint` предназначена обработки события освобождения точки сохранения.

### Функция `rollbackSavepoint`

```cpp
void rollbackSavepoint(Status* status)
```

Функция `rollbackSavepoint` предназначена обработки события отката точки сохранения.

### Функция `insertRecord`

```cpp
void insertRecord(Status* status, const char* name, IStreamedRecord* record)
```

Функция `insertRecord` предназначена обработки события вставки новой записи в таблицу. В параметр `name` передаётся имя таблицы. В параметр `record` передаётся указатель на интерфейс `IStreamedRecord`, предназначенный для доступа к значениям полей вставляемой записи.

### Функция `updateRecord`

```cpp
void updateRecord(Status* status, const char* name, IStreamedRecord* orgRecord, IStreamedRecord* newRecord)
```

Функция `updateRecord` предназначена обработки события обновления записи в таблице. В параметр `name` передаётся имя таблицы. В параметр `orgRecord` передаётся указатель на интерфейс `IStreamedRecord`, предназначенный для доступа к старым значениям полей обновляемой записи. В параметр `newRecord` передаётся указатель на интерфейс `IStreamedRecord`, предназначенный для доступа к новым значениям полей обновляемой записи.

### Функция `deleteRecord`

```cpp
void deleteRecord(Status* status, const char* name, IStreamedRecord* record)
```

Функция `deleteRecord` предназначена обработки события удаления записи из таблицы. В параметр `name` передаётся имя таблицы. В параметр `record` передаётся указатель на интерфейс `IStreamedRecord`, предназначенный для доступа к значениям полей удаляемой записи.

### Функция `executeSql`

```cpp
void executeSql(Status* status, const char* sql)
```

Функция `executeSql` предназначена обработки события выполнения SQL запроса (обычно это DDL запросы для модификации метаданных базы данных). В параметр `sql` передаётся текст SQL запроса. SQL запрос будет в кодировки ANSI или UTF8.

### Функция `executeSqlIntl`

```cpp
void executeSqlIntl(Status* status, unsigned charset, const char* sql)
```

Функция `executeSqlIntl` предназначена обработки события выполнения SQL запроса (обычно это DDL запросы для модификации метаданных базы данных). В отличии от предыдущей функции здесь учитывается кодировка (параметр `charset`) в которой был передан запрос. В параметр `sql` передаётся текст SQL запроса.

### Функция `storeBlob`

```cpp
void storeBlob(Status* status, ISC_QUAD* blob_id,
            ISC_INT64 length, const unsigned char* data)
```

Функция `storeBlob` предназначена обработки события сохранения BLOB данных. В параметр `blob_id` передаётся идентификатор BLOB, `length` - длина данных в байтах, `data` - указатель на данные.

---

## Интерфейс `IStreamPluginFactory`

Интерфейс `IStreamPluginFactory` - содержит функции для создания экземпляра класса интерфейса `IStreamPlugin`.

### Функция `createPlugin`

```cpp
IStreamPlugin* createPlugin(Status* status, IConfig* config, IStringEncodeUtils* encodeUtils, IStreamLogger* logger)
```

Функция `createPlugin` предназначена для создания экземпляра класса интерфейса `IStreamPlugin`. Функция `createPlugin` возвращает указатель на вновь создаваемый экземпляр. В параметр `config` передаётся указатель на интерфейс для чтения конфигурации плагина. В параметр `logger` передаётся указатель на интерфейс логирования службы `fb_streaming`.

В параметр `encodeUtils` передаётся указатель на служебный интерфейс, которые помогает перекодировать строки в различные кодировки. Дело в том, что в сегментах репликации поля таблиц с символьными данными могут хранится в различных кодировках (как пользователь определил их в таблице). При написании плагина обыно требуется оперировать строкой в одной конкретной кодировке, например UTF-8. Конечно же вы сами можете реализовать перекодировку строк, но посколльку служба `fb_streaming` в любом случае загружает библиотеку `icu` мы решили предоставить специальный интерфейс упрощающий эту задачу.

> [!WARNING]
> Перед возвратом указателя на экземпляр класса интерфейса `IStreamPlugin` не забудьте увеличить счётчик ссылок с помощью вызова `addRef()`.

---

## Интерфейс `IStreamedRecord`

Интерфейс `IStreamedRecord` предназначен для доступа к значениям полей новых или старых записей.

### Функция `getCount`

```cpp
unsigned getCount()
```

Функция `getCount` возвращает количество полей в записи.

### Функция `getField`

```cpp
IStreamedField* getField(unsigned index)
```

Функция `getField` возвращает указатель на интерфейс `IStreamedField`, который используется для доступа к данным конкретного поля, кроме того, с его помощью можно получить информацию о типе поля, его наименование, размеры и другие атрибуты. В параметр `index` передаётся индекс поля в записи.

### Функция `getRawLength`

```cpp
unsigned getRawLength()
```

Функция `getRawLength` возвращает размер записи в байтах.

### Функция `getRawData`

```cpp
const unsigned char* getRawData()
```

Функция `getRawData` возвращает указатель на "сырые" данные записи.

---

## Интерфейс `IStreamedField`

Интерфейс `IStreamedField` предназначен для доступа к данным конкретного поля.  Кроме того, с его помощью можно получить информацию о типе поля, его наименование, размеры и другие атрибуты.

### Функция `getName`

```cpp
const char* getName()
```

Функция `getName` возвращает имя поля.

### Функция `getType`

```cpp
unsigned getType()
```

Функция `getType` возвращает тип поля.

### Функция `getSubType`

```cpp
int getSubType()
```

Функция `getSubType` возвращает под тип поля (для полей типа `BLOB`).

### Функция `getScale`

```cpp
int getScale()
```

Функция `getScale` возвращает масштаб поля (для полей типа `NUMERIC` и `DECIMAL`).

### Функция `getLength`

```cpp
unsigned getLength()
```

Функция `getLength` возвращает размер буфера данных выделенного под значение поля.

### Функция `getCharSet`

```cpp
unsigned getCharSet()
```

Функция `getCharSet` возвращает кодировку поля (для полей типов `CHAR`, `VARCHAR` и `BLOB`).

### Функция `getData`

```cpp
const void* getData()
```

Функция `getData` возвращает указатель на значение поля. Если поле имеет значение `NULL`, то возвращается `nullptr`.

### Функция `isKey`

```cpp
FB_BOOLEAN isKey()
```

Функция `isKey` возвращает входит ли данное поле в первичный ключ таблицы.

### Функция `keyPosition`

```cpp
unsigned keyPosition()
```

Функция `keyPosition` возвращает позицию поля в первичном ключе, если оно входит в первичный ключ, в остальных случаях - возвращает 0.

---

## Интерфейс `IStreamLogger`

Интерфейс `IStreamLogger` предназначен для записи сообщений в журнала работы службы `fb_streaming`. Этот интерфейс удобен для отладки плагина.

Уровни логирования описаны в константах интерфейса `IStreamLogger`:

* `IStreamLogger::LEVEL_TRACE`
* `IStreamLogger::LEVEL_DEBUG`
* `IStreamLogger::LEVEL_INFO`
* `IStreamLogger::LEVEL_WARN`
* `IStreamLogger::LEVEL_ERROR`
* `IStreamLogger::LEVEL_CRITICAL`

Минимальный уровень логирования сообщений настраивается в конфигурационном файле `fb_streaming.conf` в параметре `logLevel`.

### Функция `getLevel`

```cpp
unsigned getLevel()
```

Функция `getLevel` возвращает уровень логирования.

### Функция `log`

```cpp
void log(unsigned level, const char* message)
```

Функция `log` предназначена для записи сообщения в журнала работы службы `fb_streaming`. Данную функцию удобно использовать для отладки работы плагина.

В параметр `level` передаётся уровень логирования.

В параметр `message` передаётся текст сообщения.

### Функция `trace`

```cpp
void trace(const char* message)
```

Функция `trace` предназначена для записи сообщения в журнала работы службы `fb_streaming` с уровнем логирования `IStreamLogger::LEVEL_TRACE`. В параметр `message` передаётся текст сообщения.

### Функция `debug`

```cpp
void debug(const char* message)
```

Функция `debug` предназначена для записи сообщения в журнала работы службы `fb_streaming` с уровнем логирования `IStreamLogger::LEVEL_DEBUG`. В параметр `message` передаётся текст сообщения.

### Функция `info`

```cpp
void info(const char* message)
```

Функция `info` предназначена для записи сообщения в журнала работы службы `fb_streaming` с уровнем логирования `IStreamLogger::LEVEL_INFO`. В параметр `message` передаётся текст сообщения.

### Функция `warning`

```cpp
void warning(const char* message)
```

Функция `warning` предназначена для записи сообщения в журнала работы службы `fb_streaming` с уровнем логирования `IStreamLogger::LEVEL_WARN`. В параметр `message` передаётся текст сообщения.

### Функция `error`

```cpp
void error(const char* message)
```

Функция `error` предназначена для записи сообщения в журнала работы службы `fb_streaming` с уровнем логирования `IStreamLogger::LEVEL_ERROR`. В параметр `message` передаётся текст сообщения.

### Функция `critical`

```cpp
void critical(const char* message)
```

Функция `critical` предназначена для записи сообщения в журнала работы службы `fb_streaming` с уровнем логирования `IStreamLogger::LEVEL_CRITICAL`. В параметр `message` передаётся текст сообщения.

---

## Интерфейс `IStringEncodeUtils`

Интерфейс `IStringEncodeUtils` предоставляет функции для преобразования строк из одной кодировки в другую. Для упрощения работы с этим интерфейсом мы написали высокоуровневую обёртку [`StringEncodeHelper`](../src/encoding/StringEncodeHelper.h).

### Функция `getConverterById`

```cpp
IStringConverter* getConverterById(Status* status, unsigned charsetId)
```

Возвращает экземляр интерфейса `IStringConverter`, который предназначен для перекодировки строк из своей кодировки в одну из кодировок unicode. Параметр `charsetId` задаёт идентификатор кодировки Firebird для перекодировщика `IStringConverter`. Идентифтикаторы кодировок Firebird перечислдены в файле [`charsets.h`](../src/common/charsets.h).

### Функция `getConverterByName`

```cpp
IStringConverter* getConverterByName(Status* status, const char* charsetName)
```

Возвращает экземляр интерфейса `IStringConverter`, который предназначен для перекодировки строк из своей кодировки в одну из кодировок unicode. Параметр `charsetName` задаёт имя набора символов Firebird для перекодировщика `IStringConverter`.

### Функция `convertCharset`

```cpp
ISC_UINT64 convertCharset(Status* status, IStringConverter* srcConveter, IStringConverter* dstConveter, 
    const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из одной кодировки в другую, используя экземпляры интерфейса `IStringConverter` перекодировщиков для исходной и целевой кодировки. Экземпляры перекодировщиков можно получить исопльзуя функции `getConverterById` или `getConverterByName`.

Параметры функции `convertCharset`:

* `status` - статус вектор;
* `srcConveter` - экземпляр перекодировщика для исходной кодировки;
* `dstConveter` - экземпляр перекодировщика для целевой кодировки;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `convertUtf8ToWCS`

```cpp
ISC_UINT64 convertUtf8ToWCS(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) 
```

Перекодирует строку из кодировки `utf-8` в строку с широкими символами (типа `wchar_t`). 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

> [!NOTE]
> **Примечание**: `wchar_t` не является платформенно независимым типом, его размер зависит от операционной системы на которой происходит компиляция.

### Функция `convertUtf8ToUtf16`

```cpp
ISC_UINT64 convertUtf8ToUtf16(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки `utf-8` в строку с кодировкой `utf-16`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `convertUtf8ToUtf32`

```cpp
ISC_UINT64 convertUtf8ToUtf32(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки `utf-8` в строку с кодировкой `utf-32`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `convertUtf16ToWCS`

```cpp
ISC_UINT64 convertUtf16ToWCS(Status* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки `utf-16` в строку с широкими символами (типа `wchar_t`). 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

> [!NOTE]
> **Примечание**: `wchar_t` не является платформенно независимым типом, его размер зависит от операционной системы на которой происходит компиляция.

### Функция `convertUtf16ToUtf8`

```cpp
ISC_UINT64 convertUtf16ToUtf8(Status* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки `utf-16` в строку с кодировкой `utf-8`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `convertUtf16ToUtf32`

```cpp
ISC_UINT64 convertUtf16ToUtf32(Status* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки `utf-16` в строку с кодировкой `utf-32`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `convertUtf32ToWCS`

```cpp
ISC_UINT64 convertUtf32ToWCS(Status* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки `utf-32` в строку с широкими символами (типа `wchar_t`). 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

> [!NOTE]
> **Примечание**: `wchar_t` не является платформенно независимым типом, его размер зависит от операционной системы на которой происходит компиляция.

### Функция `convertUtf32ToUtf8`

```cpp
ISC_UINT64 convertUtf32ToUtf8(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки `utf-32` в строку с кодировкой `utf-8`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `convertUtf32ToUtf16`

```cpp
ISC_UINT64 convertUtf32ToUtf16(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки `utf-32` в строку с кодировкой `utf-16`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `convertWCSToUtf8`

```cpp
ISC_UINT64 convertWCSToUtf8(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку с широкими символами (типа `wchar_t`) в строку с кодировкой `utf-8`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

> [!NOTE]
> **Примечание**: `wchar_t` не является платформенно независимым типом, его размер зависит от операционной системы на которой происходит компиляция.

### Функция `convertWCSToUtf16`

```cpp
ISC_UINT64 convertWCSToUtf16(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку с широкими символами (типа `wchar_t`) в строку с кодировкой `utf-16`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

> [!NOTE]
> **Примечание**: `wchar_t` не является платформенно независимым типом, его размер зависит от операционной системы на которой происходит компиляция.

### Функция `convertWCSToUtf32`

```cpp
ISC_UINT64 convertWCSToUtf32(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку с широкими символами (типа `wchar_t`) в строку с кодировкой `utf-32`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

> [!NOTE]
> **Примечание**: `wchar_t` не является платформенно независимым типом, его размер зависит от операционной системы на которой происходит компиляция.

----

## Интерфейс `IStringConverter`

Интерфейс `IStringConverter` предоставляет функции для преобразования строк из кодировки, привязанной к экземляру этого интерфейса, в одну из кодировок unicode. Для упрощения работы с этим интерфейсом мы написали высокоуровневую обёртку [`StringConverterHelper`](../src/encoding/StringConverterHelper.h).

### Функция `getMaxCharSize`

```cpp
int getMaxCharSize()
```

Возвращет максимальный размер символа в байтах для кодировки привязанной к экземпляру интерфейса `IStringConverter`.

### Функция `getMinCharSize`

```cpp
int getMinCharSize()
```

Возвращет минимальный размер символа в байтах для кодировки привязанной к экземпляру интерфейса `IStringConverter`.

### Функция `getCharsetId`

```cpp
unsigned getCharsetId()
```

Возвращает идентифтикатор кодировки Firebird привязанной к экземпляру интерфейса `IStringConverter`.

Идентифтикаторы кодировок Firebird перечислдены в файле [`charsets.h`](../src/common/charsets.h).

### Функция `getCharsetName`

```cpp
const char* getCharsetName()
```

Возвращает наименование набора символов Firebird привязанному к экземпляру интерфейса `IStringConverter`.

### Функция `toUtf8`

```cpp
ISC_UINT64 toUtf8(Status* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки привязанной к экземпляру интерфейса `IStringConverter` в строку с кодировкой `utf-8`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `fromUtf8`

```cpp
ISC_UINT64 fromUtf8(Status* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки `utf-8` в строку с кодировкой привязанной к экземпляру интерфейса `IStringConverter`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `toUtf16`

```cpp
ISC_UINT64 toUtf16(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) 
```

Перекодирует строку из кодировки привязанной к экземпляру интерфейса `IStringConverter` в строку с кодировкой `utf-16`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `fromUtf16`

```cpp
ISC_UINT64 fromUtf16(Status* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)  
```

Перекодирует строку из кодировки `utf-16` в строку с кодировкой привязанной к экземпляру интерфейса `IStringConverter`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `toUtf32`

```cpp
ISC_UINT64 toUtf32(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)  
```

Перекодирует строку из кодировки привязанной к экземпляру интерфейса `IStringConverter` в строку с кодировкой `utf-32`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `fromUtf32`

```cpp
ISC_UINT64 fromUtf32(Status* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Перекодирует строку из кодировки `utf-32` в строку с кодировкой привязанной к экземпляру интерфейса `IStringConverter`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

### Функция `toWCS`

```cpp
ISC_UINT64 toWCS(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize) 
```

Перекодирует строку из кодировки привязанной к экземпляру интерфейса `IStringConverter` в строку с широкими символами (типа `wchar_t`). 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

> [!NOTE]
> **Примечание**: `wchar_t` не является платформенно независимым типом, его размер зависит от операционной системы на которой происходит компиляция.

### Функция `fromWCS`

```cpp
ISC_UINT64 fromWCS(Status* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize) 
```

Перекодирует строку с широкими символами (типа `wchar_t`) в строку с кодировкой привязанной к экземпляру интерфейса `IStringConverter`. 

Параметры функции:

* `status` - статус вектор;
* `src` - исходная строка;
* `srcSize` - размер исходной строки;
* `destBuffer` - буфер для размещения целевой строки;
* `destBufferSize` - размер буфера для целевой строки.

Функция возращает длину целевой строки, если перекодировка удалась.

> [!NOTE]
> **Примечание**: `wchar_t` не является платформенно независимым типом, его размер зависит от операционной системы на которой происходит компиляция.

----
