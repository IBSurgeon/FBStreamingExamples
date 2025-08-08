# Утилита fb_repl_print

Утилита `fb_repl_print` позволяет просматривать содержимое указанного файл журнала репликации в понятной для человека форме. Сами журнала репликации имеют бинарный формат, поэтому производить их анализ без специальной утилиты невозможно.

В файле журнала репликации могут быть следующие события:

- старт транзакции (START);
- выполнение первой фазы подтверждения двухфазной транзакции (PREPARE);
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

```bash
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
    -R [ --dump-records ]                Dump table records
                                            (require --database option)
    -F [ --print-format ]                Print record format
                                            (require --dump-records option)

Database options:
    -d [ --database ] connection_string  Database connection string
    -u [ --username ] user               User name
    -p [ --password ] password           Password
```

Единственным обязательным параметром для работы утилиты `fb_repl_print` является путь к файлу журнала репликации. Он может быть задан либо первым аргументом, либо с помощью опции `--input-file` или `-i`. В этом режиме утилита выведет на экран информацию обо всех событиях присутствующих в файле журнала репликации.

Пример использования:

```
fb_repl_print "d:\fbdata\test\archive\TEST.FDB.journal-000000001"

BLOCK (offset: 48)
[200] START (offset: 64)
[200] SAVE (offset: 65)
[200] SAVE (offset: 66)
[200] INSERT CUSTOMERS (length: 3082) (offset: 78)
[200] RELEASE (offset: 3169)
[200] RELEASE (offset: 3170)
BLOCK (offset: 3171)
[] SET SEQUENCE RDB$2 = 1 (offset: 3194)
[200] COMMIT (offset: 3207)
```

Здесь `200` в квадратных скобках это номер транзакции в которой производятся действия. Если в квадратных скобках пусто, то действие происходит вне механизма контроля транзакций.

`offset` обозначает смещение в байтах относительно начала файла сегмента. Эта информация может помочь в поисках ошибок репликации.

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
fb_repl_print "d:\fbdata\test\archive\TEST.FDB.journal-000000001" -H

================================================================================
Segment: TEST.FDB.journal-000000001
Version: 1
Guid: {9D66A972-A8B9-42E0-8542-82D1DA5F1692}
Sequence: 1
State: archive
Length: 3208
================================================================================


BLOCK (offset: 48)
[200] START (offset: 64)
[200] SAVE (offset: 65)
[200] SAVE (offset: 66)
[200] INSERT CUSTOMERS (length: 3082) (offset: 78)
[200] RELEASE (offset: 3169)
[200] RELEASE (offset: 3170)
BLOCK (offset: 3171)
[] SET SEQUENCE RDB$2 = 1 (offset: 3194)
[200] COMMIT (offset: 3207)
```

Если в таблице содержатся BLOB поля, то при операциях INSERT и UPDATE новые значения этих полей пишутся в базу данных до того, как будет выполнена вставка или обновление записи. Затем, когда происходит непосредственная вставка или обновление записи, в BLOB поле записывается ссылка на ранее сохранённое значение. Эти действия дублируются также и в журнале репликации. Для того чтобы просмотреть содержимое этих сохранённых BLOB, воспользуйтесь переключателем `-B` или `--print-blobs`.

Пример печати BLOB из журнала репликации:

```
fb_repl_print "d:\fbdata\4.0\replication\fts_demo\archive\FTS_DEMO.FDB.journal-000000003" -B

BLOCK (offset: 48)
[605] SAVE (offset: 65)
[605] SAVE (offset: 66)
[605] BLOB 131:1110727 (length 113) (offset: 67)
4d616b652073757265207468697320666974732062792065...
[605] BLOB 131:1110727 (length 0) (offset: 191)
[605] BLOB 131:1110728 (length 102) (offset: 193)
4153494e3a423030425941515954517c5368697070696e67...
[605] BLOB 131:1110728 (length 0) (offset: 306)
[605] BLOB 131:1110729 (length 802) (offset: 308)
476f20746f20796f7572206f726465727320616e64207374617274...
[605] BLOB 131:1110729 (length 0) (offset: 1121)
[605] INSERT PRODUCTS (length: 4539) (offset: 1133)
[605] RELEASE (offset: 5681)
[605] RELEASE (offset: 5682)
BLOCK (offset 5683)
[] SET SEQUENCE RDB$7 = 10022 (offset: 5686)
[605] COMMIT (offset: 5699)
```

Расшифруем строку `[605] BLOB 131:1110727 (length 113)`. Число `605` - это номер транзакции, далее следует имя события `BLOB`, что обозначает сохранить BLOB.
После чего следуют идентификатор BLOB в формате `<number>:<number>`. Далее в круглых скобках указывается длина сегмента BLOB в байтах. Следующей строкой следует содержимое сегмента BLOB (порядок байтов) в 16-ричном представлении. Таких секций с одинаковым идентификатор BLOB может быть несколько (для BLOB длиннее 65535 байт или сегментированных BLOB с меньше размером сегмента). Заканчивается событие сохранения BLOB той же строкой, которой оно начиналось, но с нулевой длинной.

> [!NOTE]
> В примерах вывод BLOB полей сокращён, на самом деле никакого троеточия в выводе нет. Все байты BLOB полей выводятся на печать.

Вместе с событиями INSERT, UPDATE и DELETE в журнале репликации всегда сохраняются новая и старая запись.

Для события INSERT - новая запись, для UPDATE - новая и старая запись, для DELETE - старая запись. Записи таблицы сохраняются в формате очень близкому к тому, как они хранятся непосредственно в базе данных. Для того чтобы расшифровать значения полей записи необходимы сведения об используемом формате. Форматы записей в базе данных хранятся в таблице `RDB$FORMATS`. В HQBird 2.5 и HQBird 3.0 протокол репликации также содержит сведения о формате записи для текущей операции (так называемый логический формат записи). Однако логические форматы не были включены в протокол Firebird 4.0 (HQBird 4.0). В принципе для применения изменений из журнала репликации на стороне реплики можно обойтись без них, поскольку на момент применения операций INSERT/UPDATE/DELETE, DDL оператор изменивший формат уже должен быть выполнен на стороне реплики, и потому формат её известен. Однако в редких случаях это правило не срабатывает, а потому могут происходить отказы. В будущих версиях это может быть изменено.

Для вывода новых и старых значений столбцов записей в событиях INSERT, UPDATE и DELETE, необходимо указать переключатель `-R` или `--dump-records`. По описанным выше причинам, этот переключатель требует также ряд других опций для того, чтобы соединиться с базой данных и прочитать сведения о формате записи. Соединение с базой данных всегда производится с набором символов UTF8.

Пример вывода значений полей записей:

```
fb_repl_print "d:\fbdata\4.0\replication\fts_demo\archive\FTS_DEMO.FDB.journal-000000004" -B -R -d inet://localhost:3054/fts_demo -u SYSDBA -p masterkey

BLOCK (offset: 48)
[698] START (offset: 64)
[698] SAVE (offset: 65)
[698] SAVE (offset: 66)
[698] BLOB 131:1920 (length 315) (offset: 67)
4d616b6520737572652074686973206669747320627920656e74...
[698] BLOB 131:1920 (length 0) (offset: 393)
[698] BLOB 131:1921 (length 315) (offset: 395)
50726f6475637444696d656e73696f6e733a3578307834696e636865737c49...
[698] BLOB 131:1921 (length 0) (offset: 721)
[698] BLOB 131:1922 (length 239) (offset: 723)
73686f7720757020746f203220726576696577732062792064656661756c742...
[698] BLOB 131:1922 (length 0) (offset: 973)
[698] UPDATE PRODUCTS (orgLength: 4539, newLength: 4539) (offset: 985)

Old Data:
{
    "PRODUCT_ID": 15,
    "PRODUCT_UUID": "1ecccb43e0f5c0162218371916ffa553",
    "ABOUT_PRODUCT": "131:1920",
    "PRODUCT_SPECIFICATION": "131:1921",
    "TECHNICAL_DETAILS": "131:1922",
    "VARIANTS": null,
    "PRODUCT_NAME": "C&D Visionary DC Comic Originals Flash Logo Sticker",
    "UPC_EAN_CODE": null,
    "SELLING_PRICE": "$4.99",
    "MODEL_NUMBER": "S-DC-0140",
    "SHIPPING_WEIGHT": "0.16 ounces",
    "PRODUCT_DIMENSIONS": null,
    "PRODUCT_URL": "https://www.amazon.com/Visionary-Comic-Originals-Flash-Sticker/dp/B00U87LZ0Y",
    "IS_AMAZON_SELLER": true
}

New Data:
{
    "PRODUCT_ID": 15,
    "PRODUCT_UUID": "1ecccb43e0f5c0162218371916ffa553",
    "ABOUT_PRODUCT": "131:1920",
    "PRODUCT_SPECIFICATION": "131:1921",
    "TECHNICAL_DETAILS": "131:1922",
    "VARIANTS": null,
    "PRODUCT_NAME": "C&D Visionary DC Comic Originals Flash Logo Sticker",
    "UPC_EAN_CODE": null,
    "SELLING_PRICE": "$4.99",
    "MODEL_NUMBER": "S-DC-0140",
    "SHIPPING_WEIGHT": "0.17 ounces",
    "PRODUCT_DIMENSIONS": null,
    "PRODUCT_URL": "https://www.amazon.com/Visionary-Comic-Originals-Flash-Sticker/dp/B00U87LZ0Y",
    "IS_AMAZON_SELLER": true
}
[698] RELEASE (offset: 10076)
[698] RELEASE (offset: 10077)
BLOCK (offset 10078)
[698] COMMIT (offset: 10094)
...
```

Значение полей записей выведены в формате JSON. Все строки выводятся как есть, за исключением строк с набором символом `OCTETS` (или типа `[VAR]BINARY`), которые выводятся в шестнадцатеричном представлении. Для BLOB полей вместо их значения выводится идентификатор, по этому идентификатору вы можете найти значения BLOB, поскольку они сохранены ранее. Обратите внимание, что значения идентификаторов BLOB полей есть как для новых, так и для старых записей, а сами значения BLOB сохраняются только для новых записей.

> [!NOTE]
> Утилита всегда выводит содержимое файлов журнала репликации в кодировке UTF8. Это обозначает? что для правильного отображения строковых символов вы должны переключить кодировку своей консоли. В Windows это делается с помощью команды `chcp 65001`.

Нужный формат записи, да и саму таблицу не всегда можно найти в базе данных. Например, вы могли выполнить команду `DROP TABLE`, которая конечно же записалась в какой-либо журнал репликации, но вы просматриваете утилитой `fb_repl_print` тот файл журнала, в котором эта таблица уже существовала.

Если таблица или формат для расшифровки значений полей записи не найден, то `fb_repl_print` выдаст соответствующее предупреждение.

Если необходимо увидеть формат записи (имена полей таблицы и их типы) добавьте переключатель `-F` или `--print-format`.

```
fb_repl_print "d:\fbdata\4.0\replication\fts_demo\archive\FTS_DEMO.FDB.journal-000000004" -B -R -F -d inet://localhost:3054/fts_demo -u SYSDBA -p masterkey

BLOCK (offset: 48)
[698] START (offset: 64)
[698] SAVE (offset: 65)
[698] SAVE (offset: 66)
[698] BLOB 131:1920 (length 315) (offset: 67)
4d616b6520737572652074686973206669747320627920656e746572696e6720796f757...
[698] BLOB 131:1920 (length 0) (offset: 393)
[698] BLOB 131:1921 (length 315) (offset: 395)
50726f6475637444696d656e73696f6e733a3578307834696e636865737c4974656d576....
[698] BLOB 131:1921 (length 0) (offset: 721)
[698] BLOB 131:1922 (length 239) (offset: 723)
73686f7720757020746f203220726576696577732062792064656661756c742043264420...
[698] BLOB 131:1922 (length 0) (offset: 973)
[698] UPDATE PRODUCTS (orgLength: 4539, newLength: 4539) (offset: 985)

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

Old Data:
{
    "PRODUCT_ID": 15,
    "PRODUCT_UUID": "1ecccb43e0f5c0162218371916ffa553",
    "ABOUT_PRODUCT": "131:1920",
    "PRODUCT_SPECIFICATION": "131:1921",
    "TECHNICAL_DETAILS": "131:1922",
    "VARIANTS": null,
    "PRODUCT_NAME": "C&D Visionary DC Comic Originals Flash Logo Sticker",
    "UPC_EAN_CODE": null,
    "SELLING_PRICE": "$4.99",
    "MODEL_NUMBER": "S-DC-0140",
    "SHIPPING_WEIGHT": "0.16 ounces",
    "PRODUCT_DIMENSIONS": null,
    "PRODUCT_URL": "https://www.amazon.com/Visionary-Comic-Originals-Flash-Sticker/dp/B00U87LZ0Y",
    "IS_AMAZON_SELLER": true
}

New Data:
{
    "PRODUCT_ID": 15,
    "PRODUCT_UUID": "1ecccb43e0f5c0162218371916ffa553",
    "ABOUT_PRODUCT": "131:1920",
    "PRODUCT_SPECIFICATION": "131:1921",
    "TECHNICAL_DETAILS": "131:1922",
    "VARIANTS": null,
    "PRODUCT_NAME": "C&D Visionary DC Comic Originals Flash Logo Sticker",
    "UPC_EAN_CODE": null,
    "SELLING_PRICE": "$4.99",
    "MODEL_NUMBER": "S-DC-0140",
    "SHIPPING_WEIGHT": "0.17 ounces",
    "PRODUCT_DIMENSIONS": null,
    "PRODUCT_URL": "https://www.amazon.com/Visionary-Comic-Originals-Flash-Sticker/dp/B00U87LZ0Y",
    "IS_AMAZON_SELLER": true
}
[698] RELEASE (offset: 10076)
[698] RELEASE (offset: 10077)
BLOCK (offset 10078)
[698] COMMIT (offset: 10094)
```

## Поиск ошибок в сегментах репликации

Предположим, что в процессе применения изменений к реплике, возникает ошибка. В этом случае в `replication.log` обычно записана причина ошибки.
Там может быть что-то вроде

```
fb-repl-server (replica) Mon Jul 29 13:15:05 2024
	Database: /var/fbdata/mydb.fdb
	ERROR: violation of FOREIGN KEY constraint "FK_TRIAL_LINE_REF_HORSE" on table "TRIAL_LINE"
	Foreign key reference target does not exist
	Problematic key value is ("CODE_HORSE" = 12345)
	At segment 106, offset 130012
```

В этом случае вы можете посмотреть содержимое сегмента `106` с помощью `fb_repl_print` и найти место возникновения ошибки по `offset` равному `130012`.

```
fb_repl_print "/var/fbdata/replication/archive/MYDB.FDB.journal-000000106" -B -R -d inet://localhost:mydb -u SYSDBA -p masterkey

...
BLOCK (offset: 130012)
[698] START (offset: 130013)
[698] SAVE (offset: 130014)
[698] SAVE (offset: 130015)
[698] INSERT TRIAL_LINE (length: 4539) (offset: 130016)

Data:
{
    "CODE_TRIAL_LINE": 155674870,
    "CODE_HORSE": 12345,
    ...
}
...

```
