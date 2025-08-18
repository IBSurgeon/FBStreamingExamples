# Плагин simple_json_plugin

Плагин `simple_json_plugin` предназначен для автоматической трансляции бинарных файлов журнала репликации в эквивалентный JSON формат
и сохранять журналы в заданную директорию с тем же именем, но с расширением `.json`.

Каждый json файл содержит в себе корневой объект, состоящий из двух полей: `header` и `events`.

Поле `header` представляет собой объект описывающий заголовок сегмента репликации. Он содержит следующие поля:

* `guid` - GUID базы данных;
* `sequence` - номер сегмента репликации;
* `state` - состояние в котором находится сегмент. Возможные значения: `free`, `used`, `full`, `archive`;
* `version` - версия протокола журнала репликации.

Поле `events` представляет собой массив объектов, каждый из которых представляет одно из событий репликации.
Объект события может содержать следующие поля:

* `event` - тип события. Доступны следующие варианты:
  * `SET SEQUENCE` - установка значения последовательности (генератора);
  * `START TRANSACTION` - старт транзакции;
  * `PREPARE TRANSACTION` - выполнение первой фазы подтверждения двухфазной транзакции;
  * `SAVEPOINT` - установка точки сохранения;
  * `RELEASE SAVEPOINT` - освобождение точки сохранения;
  * `ROLLBACK SAVEPOINT` - откат точки сохранения;
  * `COMMIT` - подтверждение транзакции;
  * `ROLLBACK` - откат транзакции;
  * `INSERT` - вставка новой записи в таблицу;
  * `UPDATE` - обновление записи в таблице;
  * `DELETE` - удаление записи из таблицы;
  * `EXECUTE SQL` - выполнение SQL оператора. Такие события происходят только для DDL операторов;
  * `STORE BLOB` - сохранение BLOB.
* `tnx` - номер транзакции. Доступно для всех событий кроме `SET SEQUENCE`, поскольку генераторы работают вне контекста транзакций;
* `sequence` - имя последовательности. Доступно только в событии `SET SEQUENCE`;
* `value` - значение последовательности. Доступно только в событии `SET SEQUENCE`;
* `sql` - текст SQL запроса. Доступно только в событии `EXECUTE SQL`;
* `blobId` - идентификатор BLOB. Доступно только в событии `STORE BLOB`;
* `data` - данные сегмента BLOB в шестнадцатеричном представлении. Доступно только в событии `STORE BLOB`;
* `table` - имя таблицы. Доступно в событиях `INSERT`, `UPDATE` и `DELETE`;
* `record` - значения полей записи. Для событий `INSERT` и `UPDATE` это новая запись, а для `DELETE` - старая;
* `oldRecord` - значения полей старой записи. Доступно только в событии `UPDATE`;
* `changedFields` - массив имён изменённых полей. Доступно только в событии `UPDATE`.

Обратите внимание, что для BLOB полей в качестве значения указан идентификатор BLOB.

Пример содержимого файла `.json`:

```json
{
    "header": {
        "version": 1,
        "guid": "{F396449D-F6E4-4812-875E-248AB7C2BEE7}",
        "sequence": 1,
        "state": "archive"
    },
    "events": [
        {
            "event": "START TRANSACTION",
            "tnx": 1937
        },
        {
            "event": "SAVEPOINT",
            "tnx": 1937
        },
        {
            "event": "SAVEPOINT",
            "tnx": 1937
        },
        {
            "event": "UPDATE",
            "table": "CUSTOMER",
            "tnx": 1937,
            "changedFields": [
                "ADDRESS"
            ],
            "oldRecord": {
                "CUSTOMER_ID": 8,
                "NAME": "Abigail Thomas",
                "ADDRESS": null,
                "ZIPCODE": null,
                "PHONE": "1-290-853-7531"
            },
            "record": {
                "CUSTOMER_ID": 8,
                "NAME": "Abigail Thomas",
                "ADDRESS": "555",
                "ZIPCODE": null,
                "PHONE": "1-290-853-7531"
            }
        },
        {
            "event": "RELEASE SAVEPOINT",
            "tnx": 1937
        },
        {
            "event": "RELEASE SAVEPOINT",
            "tnx": 1937
        },
        {
            "event": "COMMIT",
            "tnx": 1937
        }
    ]
}
```

## Настройка плагина

Пример настройки плагина:

```
task = d:\fbdata\4.0\replication\examples\json_source
{
    deleteProcessedFile = true
    database = inet://localhost:3054/examples
    username = SYSDBA
    password = masterkey
    plugin = simple_json_plugin
    dumpBlobs = true
    register_ddl_events = true
    register_sequence_events = true
    outputDir = d:\fbdata\4.0\replication\examples\json_archive
    # include_tables =
    # exclude_tables =
}
```

Описание параметров:

* `controlFileDir` - директория в которой будет создан контрольный файл (по умолчанию та же директория, что и `sourceDir`);
* `database` - строка подключения к базе данных (обязательный);
* `username` - имя пользователя для подключения к базе данных;
* `password` - пароль для подключения к базе данных;
* `plugin` - плагин, который обрабатывает события, возникающие в процессе анализа журнала репликации (обязательный);
* `deleteProcessedFile` - удалять ли файл журнала после обработки (по умолчанию `true`);
* `outputDir` - директория в которой будут находится готовые JSON файлы (обязательный);
* `dumpBlobs` - публиковать ли данные BLOB полей (по умолчанию `false`);
* `register_ddl_events` - регистрировать ли DDL события (по умолчанию `true`);
* `register_sequence_events` - регистрировать ли события установки значения последовательности (по умолчанию `true`);
* `include_tables` - регулярное выражение, определяющие имена таблиц для которых необходимо отслеживать события;
* `exclude_tables` - регулярное выражение, определяющие имена таблиц для которых не надо отслеживать события.
