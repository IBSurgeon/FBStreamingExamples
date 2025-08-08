# Firebird Streaming Examples

Здесь собраны примеры написания плагинов для технологии Firebird streaming.

Firebird streaming - это технология асинхронной публикации событий возникающих в процессе анализа журнала репликации.

Для обработки событий используется служба (демон) `fb_streaming`. Служба отслеживает новые файлы журнала репликации, анализирует их, и генерирует события, которые обрабатываются одним из плагинов. Подробное описание службы `fb_streaming` находится в документе [Служба fb_streaming](doc/fb_streaming_ru.md).

Кроме того, существует специальная утилита `fb_repl_print`, которая позволяет просматривать содержимое указанного файл журнала репликации, что позволяет производить отладку плагинов и непосредственно самой репликации. Подробное описание утилиты `fb_repl_print` находится в документе [Утилита fb_repl_print](doc/fb_repl_print_ru.md).

Для получения бинарных файлов службы `fb_streaming` и утилиты `fb_repl_print` обратитесь в компанию IBSurgeon.

Процесс написание собственных плагинов подробно описан в документе [Написание собственного плагина для службы fb_streaming](doc/writing_plugin_ru.md).

В качестве примера плагина рассмотрим простую библиотеку, которая будет переводить исходные бинарные файлы журнала репликации в эквивалентный JSON формат и сохранять журналы в заданную директорию с тем же именем, но с расширением `.json`. Назовём данный плагин `SimpleJsonPlugin`.

Скачать скомпилированный пример с конфигурацией можно по ссылкам:

* [SimpleJsonPlugin_Win_x64.zip](https://github.com/IBSurgeon/FBStreamingExamples/releases/download/1.0/SimpleJsonPlugin_Win_x64.zip)
* [SimpleJsonPlugin_Win_x86.zip](https://github.com/IBSurgeon/FBStreamingExamples/releases/download/1.0/SimpleJsonPlugin_Win_x64.zip)


## Описание плагина SimpleJsonPlugin

Плагин `SimpleJsonPlugin` предназначен для автоматической трансляции бинарных файлов журнала репликации в эквивалентный JSON формат 
и сохранять журналы в заданную директорию с тем же именем, но с расширением `.json`.

Каждый json файл содержит в себе корневой объект, состоящий из двух полей: `header` и `events`.

Поле `header` представляет собой объект описывающий заголовок сегмента репликации. Он содержит следующие поля:

* `guid` - GUID базы данных;
* `sequence` - номер сегмента репликации;
* `state` - состояние в котором находится сегмент. Возможные значения: `free`, `used`, `full`, `archive`;
* `version` - версия протокола журнала репликации.

Поле `events` представляет собой массив объектов, каждый из которых представляет одно из событий репликации. 
Объект события может соджержать следующие поля:

* `event` - тип события. Доступны следующие варианты:
  - `SET SEQUENCE` - установка значения последовательности (генератора);
  - `START TRANSACTION` - старт транзакции;
  - `PREPARE TRANSACTION` - выполнение первой фазы подтверждения двухфазной транзакции;
  - `SAVEPOINT` - установка точки сохранения;
  - `RELEASE SAVEPOINT` - освобождение точки сохранения;
  - `ROLLBACK SAVEPOINT` - откат точки сохранения;
  - `COMMIT` - подтверждение транзакции;
  - `ROLLBACK` - откат транзакции;
  - `INSERT` - вставка новой записи в таблицу;
  - `UPDATE` - обновление записи в таблице;
  - `DELETE` - удаление записи из таблицы;
  - `EXECUTE SQL` - выполнение SQL оператора. Такие события происходят только для DDL операторов;
  - `STORE BLOB` - сохранение BLOB.
* `tnx` - номер транзакции. Доступно для всех событий кроме `SET SEQUENCE`, поскольку генераторы работают вне контекста транзакций;
* `sequence` - имя последовательности. Доступно только в событии `SET SEQUENCE`;
* `value` - значение последжостальности. Доступно только в событии `SET SEQUENCE`;
* `sql` - текст SQL запроса. Доступно только в событии `EXECUTE SQL`;
* `blobId` - идентификатор BLOB. Доступно только в событии `STORE BLOB`;
* `data` - данные сегмента BLOB в 16-ричном представлении. Доступно только в событии `STORE BLOB`;
* `table` - имя таблицы. Доступно в событиях `INSERT`, `UPDATE` и `DELETE`;
* `record` - значения полей записи. Для событий `INSERT` и `UPDATE` это новая запись, а для `DELETE` - старая;
* `oldRecord` - значения полей старой записи. Доступно только в событии `UPDATE`;
* `changedFields` - массив имён изменённых полей. Доступно только в событии `UPDATE`.

Обратите внимание, что для BLOB полей в качестве значения указан идентификатор BLOB.

Пример содержимого файла `.json`:

```json
{
    "events": [
        {
            "event": "START TRANSACTION",
            "tnx": 6259
        },
        {
            "event": "SAVEPOINT",
            "tnx": 6259
        },
        {
            "event": "SAVEPOINT",
            "tnx": 6259
        },
        {
            "changedFields": [
                "SHORTNAME_EN"
            ],
            "event": "UPDATE",
            "oldRecord": {
                "CODE_COLOR": 3,
                "CODE_SENDER": 1,
                "NAME": "красно-серая",
                "NAME_DE": "",
                "NAME_EN": "red grey",
                "SHORTNAME": "кр.-сер.",
                "SHORTNAME_EN": ""
            },
            "record": {
                "CODE_COLOR": 3,
                "CODE_SENDER": 1,
                "NAME": "красно-серая",
                "NAME_DE": "",
                "NAME_EN": "red grey",
                "SHORTNAME": "кр.-сер.",
                "SHORTNAME_EN": "fff"
            },
            "table": "COLOR",
            "tnx": 6259
        },
        {
            "event": "RELEASE SAVEPOINT",
            "tnx": 6259
        },
        {
            "event": "RELEASE SAVEPOINT",
            "tnx": 6259
        },
        {
            "event": "COMMIT",
            "tnx": 6259
        }
    ],
    "header": {
        "guid": "{AA08CB53-C875-4CA3-B513-877D0668885D}",
        "sequence": 3,
        "state": "archive",
        "version": 1
    }
}
```

### Настройка плагина SimpleJsonPlugin

Пример настройки плагина:

```conf
task = d:\fbdata\4.0\replication\testdb\archive
{
	deleteProcessedFile = true
	database = inet://localhost:3054/test
	username = SYSDBA
	password = masterkey
	plugin = SimpleJsonPlugin
	dumpBlobs = true
	register_ddl_events = true
	register_sequence_events = true
	outputDir = d:\fbdata\4.0\replication\testdb\json_archive
	# include_tables = 
	# exclude_tables = 
}
```

Описание параметров:

* `lockDir` - директория в которой будет создан файл блокировок (по умолчанию та же директория, что и `archiveDir`);
* `database` - строка подключения к базе данных (обязательный);
* `username` - имя пользователя для подключения к базе данных;
* `password` - пароль для подключения к базе данных;
* `plugin` - плагин, который обрабатывает события, возникающие в процессе анализа журнала репликации (обязательный);
* `deleteProcessedFile` - удалять ли файл журнала после обработки (по умолчанию `true`);
* `outputDir` - директория в которой будут находится готовые JSON файлы;
* `dumpBlobs` - публиковать ли данные BLOB полей (по умолчанию false);
* `register_ddl_events` - регистрировать ли DDL события (по умолчанию true);
* `register_sequence_events` - регистрировать ли события установки значения последовательности (по умолчанию true);
* `include_tables` - регулярное выражение, определяющие имена таблиц для которых необходимо отслеживать события;
* `exclude_tables` - регулярное выражение, определяющие имена таблиц для которых не надо отслеживать события.

