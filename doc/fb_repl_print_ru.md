# Утилита fb_repl_print

Утилита `fb_repl_print` позволяет просматривать содержимое указанного файл журнала репликации в понятной для человека форме.
Сами журнала репликации имеют бинарный формат, поэтому производить их анализ без специальной утилиты невозможно.

В файле журнала репликации могут быть следующие события:

- старт транзакции (START);
- подготовка двухфазной транзакции (PREPARE);
- подтверждение транзакции (COMMIT);
- откат транзакции (ROLLBACK);
- установка точки сохранения (SAVE);
- откат точки сохранения (UNDO);
- освобождение точки сохранения (RELEASE);
- установка значения генератора (SET SEQUENCE);
- выполнение SQL оператора (EXECUTE SQL). Такие события происходят только для DDL операторов;
- сохранение BLOB (BLOB);
- вставка новой записи в таблицу (INSERT);
- обновление записи в таблице (UPDATE);
- удаление записи из таблицы (DELETE).

Информация об этих событиях, будет выведена в печатном виде утилитой `fb_repl_print`. Кроме того, с помощью дополнительных переключателей можно также вывести информацию о заголовке файла репликации, содержимое BLOB (в шестнадцатеричном виде), и значения полей для новых (события INSERT, UPDATE) и старых (события UPDATE, DELETE) записей.

Для получения справки об опциях утилиты наберите команду

```
fb_repl_print --help
```

В результате на консоль будет выведено следующее:

```
Usage fb_repl_print [file_path] <options>
General options:
  -h [ --help ]                        Show help
  -z [ --version ]                     Show version
  -i [ --input-file ] file_path        Path to the replication log file
                                       (required)
  -H [ --print-header ]                Print segment header
  -B [ --print-blobs ]                 Print BLOBs
  -R [ --dump-records ]                Dump table records (required --database
                                       option)

Database options:
  -d [ --database ] connection_string  Database connection string
  -u [ --username ] user               User name
  -p [ --password ] password           Password
```

Единственным обязательным параметром для работы утилиты `fb_repl_print` является путь к файлу журнала репликации. 
Он может быть задан либо первым аргументом, либо с помощью опции `--input-file` или `-i`. В этом режиме утилита выведет на экран информацию обо всех событиях присутствующих в файле журнала репликации.

Пример использования:

```
fb_repl_print "d:\fbdata\test\archive\TEST.FDB.journal-000000001"

[76] START
[76] SAVE
[76] INSERT RDB$BACKUP_HISTORY (length: 325)
[76] RELEASE
[] SET SEQUENCE RDB$BACKUP_HISTORY = 5
[76] COMMIT
```

Здесь `76` в квадратных скобках это номер транзакции в которой производятся действия. Если в квадратных скобках пусто, то действие происходит вне механизма контроля транзакций.

Если необходимо, чтобы так же присутствовала информация о заголовке файла журнала репликации добавите опцию `-H` или `--print-header`.
Информация в заголовке включает в себя:

- имя файла сегмента;
- версия протокола репликации;
- уникальный идентификатор базы данных (Guid);
- номер последовательности (номер файла журнала репликации);
- состояние сегмента репликации;
- размер сегмента репликации в байтах.

Пример содержимого файла журнала репликации с заголовком

```
fb_repl_print "d:\fbdata\test\archive\TEST.FDB.journal-000000001" --print-header

================================================================================
Segment: E2.FDB.journal-000000001
Version: 1
Guid: {667C3163-EC46-434C-9B2A-726A20248947}
Sequence: 1
State: archive
Length: 471
================================================================================

[76] START
[76] SAVE
[76] INSERT RDB$BACKUP_HISTORY (length: 325)
[76] RELEASE
[] SET SEQUENCE RDB$BACKUP_HISTORY = 5
[76] COMMIT
```

Если в таблице содержатся BLOB поля, то при операциях INSERT и UPDATE новые значения этих полей пишутся в базу данных до того, как будет выполнена вставка или обновление записи. Затем, когда происходит непосредственная вставка или обновление записи, в BLOB поле записывается ссылка на ранее сохранённое значение. Эти действия дублируются также и в журнале репликации. Для того чтобы просмотреть содержимое этих сохранённых BLOB, воспользуйтесь переключателем `-B` или `--print-blobs`.

Пример печати BLOB из журнала репликации:

```
fb_repl_print "d:\fbdata\4.0\replication\fts_demo\archive\FTS_DEMO.FDB.journal-000000003" -B
[605] START
[605] SAVE
[605] SAVE
[605] BLOB 131:1110727 (length 113)
4d616b6520737572652074686973206669747320627920656e746572696e6720796f7572206d6f64656c206e756d6265722e207c2032323520636f6c6f722d636f646564206361726473207c20332e3522207820322e323522206361726473207c20466f7220322d3620706c6179657273
[605] BLOB 131:1110727 (length 0)
[605] INSERT PRODUCTS (length: 4539)
[605] RELEASE
[605] RELEASE
[] SET SEQUENCE RDB$7 = 10022
[605] COMMIT
```

Расшифруем строку `[605] BLOB 131:1110727 (length 113)`. Число `605` - это номер транзакции, далее следует имя события `BLOB`, что обозначает сохранить BLOB. 
Далее в круглых скобках указывается длина BLOB в байтах. Следующей строкой следует содержимое BLOB (порядок байтов) в 16-ричном представлении.
Заканчивается событие сохранения BLOB той же строкой, которой оно начиналось, но с нулевой длинной.

> **Note** <br>
> Я не знаю для чего это сделано, но именно так это передаётся в протоколе репликации.

Вместе с событиями INSERT, UPDATE и DELETE в журнале репликации всегда сохраняются новая и старая запись. 

Для события INSERT - новая запись, для UPDATE - новая и старая запись, для DELETE - старая запись. Записи таблицы сохраняются в формате очень близкому к тому, как они хранятся непосредственно в базе данных. Для того чтобы расшифровать значения полей записи необходимы сведения об используемом формате. Форматы записей в базе данных хранятся в 
таблице `RDB$FORMATS`. В HQBird 2.5 и HQBird 3.0 протокол репликации также содержит сведения о формате записи для текущей операции (так называемый логический формат записи). Однако логические форматы не были включены в протокол Firebird 4.0 (HQBird 4.0).
В принципе для применения изменений из журнала репликации на стороне реплики можно обойтись без них, поскольку на момент применения операций INSERT/UPDATE/DELETE, DDL оператор изменивший формат уже должен быть выполнен на стороне реплики, и потому формат её известен. 
Однако в редких случаях это правило не срабатывает, а потому могут происходить отказы. В будущих версиях это может быть изменено.

Для вывода новых и старых значений столбцов записей в событиях INSERT, UPDATE и DELETE, необходимо указать переключатель `-R` или `--dump-records`.
По описанным выше причинам, этот переключатель требует также ряд других опций для того, чтобы соединиться с базой данных и прочитать сведения о формате записи. 
Соединение с базой данных всегда производится с набором символов UTF8. 

Пример вывода значений полей записей:

```
fb_repl_print "d:\fbdata\4.0\replication\fts_demo\archive\FTS_DEMO.FDB.journal-000000003" -B -R -d inet://localhost:3054/fts_demo -u SYSDBA -p masterkey
[605] START
[605] SAVE
[605] SAVE
[605] BLOB 131:1110727 (length 113)
4d616b6520737572652074686973206669747320627920656e746572696e6720796f7572206d6f64656c206e756d6265722e207c2032323520636f6c6f722d636f646564206361726473207c20332e3522207820322e323522206361726473207c20466f7220322d3620706c6179657273
[605] BLOB 131:1110727 (length 0)
[605] BLOB 131:1110728 (length 102)
4153494e3a423030425941515954517c5368697070696e675765696768743a31302e346f756e63657328566965777368697070696e677261746573616e64706f6c6963696573297c446174654669727374417661696c61626c653a417072696c312c32303133
[605] BLOB 131:1110728 (length 0)
[605] INSERT PRODUCTS (length: 4539)

Record format (fields: 14):
--------------------------------------------------------------------------------
  Id                            Name                  Type         Character set
--------------------------------------------------------------------------------
   0                      PRODUCT_ID                BIGINT
   1                    PRODUCT_UUID            BINARY(16)
   2                   ABOUT_PRODUCT    BLOB SUB_TYPE TEXT                  UTF8
   3           PRODUCT_SPECIFICATION    BLOB SUB_TYPE TEXT                  UTF8
   4               TECHNICAL_DETAILS    BLOB SUB_TYPE TEXT                  UTF8
   5                        VARIANTS    BLOB SUB_TYPE TEXT                  UTF8
   6                    PRODUCT_NAME          VARCHAR(200)                  UTF8
   7                    UPC_EAN_CODE          VARCHAR(150)                  UTF8
   8                   SELLING_PRICE          VARCHAR(400)                  UTF8
   9                    MODEL_NUMBER           VARCHAR(45)                  UTF8
  10                 SHIPPING_WEIGHT           VARCHAR(15)                  UTF8
  11              PRODUCT_DIMENSIONS           VARCHAR(50)                  UTF8
  12                     PRODUCT_URL          VARCHAR(255)                  UTF8
  13                IS_AMAZON_SELLER               BOOLEAN
--------------------------------------------------------------------------------

Data:
{
    "ABOUT_PRODUCT": "131:1110727",
    "IS_AMAZON_SELLER": true,
    "MODEL_NUMBER": "EP63759",
    "PRODUCT_DIMENSIONS": null,
    "PRODUCT_ID": 10013,
    "PRODUCT_NAME": "Edupress Splat Game, Addition (EP63759)",
    "PRODUCT_SPECIFICATION": "131:1110728",
    "PRODUCT_URL": "https://www.amazon.com/Edupress-Splat-Game-Addition-EP63759/dp/B00BYAQYTQ",
    "PRODUCT_UUID": "79156b1ec8834190a5ac9b43272e5bc0",
    "SELLING_PRICE": "$12.99",
    "SHIPPING_WEIGHT": "10.4 ounces",
    "TECHNICAL_DETAILS": null,
    "UPC_EAN_CODE": null,
    "VARIANTS": null
}
```

Значение полей записей выведены в формате JSON. Все строки выводятся как есть, за исключением строк с набором 
символом `OCTETS` (или типа `[VAR]BINARY`), которые выводятся в шестнадцатеричном представлении. Для BLOB полей 
вместо их значения выводится идентификатор, по этому идентификатору вы можете найти значения BLOB, поскольку они 
сохранены ранее. Обратите внимание, что значения идентификаторов BLOB полей есть как для новых, так и для старых 
записей, а сами значения BLOB сохраняются только для новых записей.

> **Note** <br>
> Утилита всегда выводит содержимое файлов журнала репликации в кодировке UTF8. Это обозначает? что для правильного отображения строковых символов вы должны переключить кодировку своей консоли. В Windows это делается с помощью команды `chcp 65001`.

Нужный формат записи, да и саму таблицу не всегда можно найти в базе данных. Например, вы могли выполнить 
команду `DROP TABLE`, которая конечно же записалась в какой-либо журнал репликации, но вы просматриваете утилитой 
`fb_repl_print` тот файл журнала, в котором эта таблица уже существовала.

Если таблица или формат для расшифровки значений полей записи не найден, то `fb_repl_print` выдаст соответствующее предупреждение.