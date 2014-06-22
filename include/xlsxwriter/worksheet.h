/*
 * libxlsxwriter
 *
 * Copyright 2014, John McNamara, jmcnamara@cpan.org. See LICENSE.txt.
 */

/**
 * @page worksheet_page The Worksheet object
 *
 * The Worksheet object represents an Excel worksheet. It handles
 * operations such as writing data to cells or formatting worksheet
 * layout.
 *
 * See @ref worksheet.h for full details of the functionality.
 *
 * @file worksheet.h
 *
 * @brief Functions related to adding data and formatting to a worksheet.
 *
 * The Worksheet object represents an Excel worksheet. It handles
 * operations such as writing data to cells or formatting worksheet
 * layout.
 *
 * A Worksheet object isn’t created directly. Instead a worksheet is
 * created by calling the workbook_add_worksheet() method from a
 * Workbook object:
 *
 * @code
 *     #include "xlsxwriter.h"
 *
 *     int main() {
 *
 *         lxw_workbook  *workbook  = new_workbook("filename.xlsx");
 *         lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);
 *
 *         worksheet_write_string(worksheet, 0, 0, "Hello Excel", NULL);
 *
 *         return workbook_close(workbook);
 *     }
 * @endcode
 *
 */
#ifndef __LXW_WORKSHEET_H__
#define __LXW_WORKSHEET_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "shared_strings.h"
#include "common.h"
#include "format.h"
#include "utility.h"

#define LXW_COL_META_MAX 128

/** Default column width in Excel */
#define LXW_DEF_COL_WIDTH 8.43

/** Default row height in Excel */
#define LXW_DEF_ROW_HEIGHT 15

/** Error codes from `worksheet_write*()` functions. */
enum lxw_write_error {
    LXW_WRITE_ERROR_NONE = 0,
    LXW_RANGE_ERROR,
    LXW_STRING_HASH_ERROR,
    LXW_STRING_LENGTH_ERROR,
    LXW_END
};

/** Data type to represent a row value.
 *
 * The maximum row in Excel is 1,048,576.
 */
typedef uint32_t lxw_row_t;

/** Data type to represent a column value.
 *
 * The maximum column in Excel is 16,384.
 */
typedef uint16_t lxw_col_t;

enum cell_types {
    NUMBER_CELL = 1,
    STRING_CELL,
    FORMULA_CELL,
    BLANK_CELL
};

/* Define the queue.h TAILQ structs for the list head types. */
TAILQ_HEAD(lxw_table_cells, lxw_cell);
TAILQ_HEAD(lxw_table_rows, lxw_row);

/**
 * @brief Options for rows and columns.
 *
 * Options struct for the worksheet_set_column() and worksheet_set_row()
 * functions.
 *
 * It has the following members but currently only the `hidden` property is
 * supported:
 *
 * * `hidden`
 * * `level`
 * * `collapsed`
 */
typedef struct lxw_row_col_options {
    /** Hide the row/column */
    uint8_t hidden;
    uint8_t level;
    uint8_t collapsed;
} lxw_row_col_options;

typedef struct lxw_col_options {
    lxw_col_t firstcol;
    lxw_col_t lastcol;
    double width;
    lxw_format *format;
    uint8_t hidden;
    uint8_t level;
    uint8_t collapsed;
} lxw_col_options;

/**
 * @brief Struct to represent an Excel worksheet.
 *
 * The members of the lxw_worksheet struct aren't modified directly. Instead
 * the worksheet properties are set by calling the functions shown in
 * worksheet.h.
 */
typedef struct lxw_worksheet {

    FILE *file;
    struct lxw_table_rows *table;

    lxw_row_t dim_rowmin;
    lxw_row_t dim_rowmax;
    lxw_col_t dim_colmin;
    lxw_col_t dim_colmax;

    lxw_sst *sst;
    char *name;

    uint32_t index;
    uint8_t active;
    uint8_t selected;
    uint8_t hidden;

    lxw_col_options **col_options;
    uint16_t col_options_max;

    double *col_sizes;
    uint16_t col_sizes_max;

    lxw_format **col_formats;
    uint16_t col_formats_max;

    uint8_t col_size_changed;

    STAILQ_ENTRY (lxw_worksheet) list_pointers;

} lxw_worksheet;

/*
 * Worksheet initialisation data.
 */
typedef struct lxw_worksheet_init_data {
    uint32_t index;
    uint8_t hidden;

    lxw_sst *sst;
    char *name;

} lxw_worksheet_init_data;

/* Struct to represent a worksheet row. */
typedef struct lxw_row {
    lxw_row_t row_num;
    double height;
    lxw_format *format;
    uint8_t hidden;
    uint8_t level;
    uint8_t collapsed;
    uint8_t changed;
    struct lxw_table_cells *cells;

    /* List pointers for queue.h. */
    TAILQ_ENTRY (lxw_row) list_pointers;
} lxw_row;

/* Struct to represent a worksheet cell. */
typedef struct lxw_cell {
    lxw_row_t row_num;
    lxw_col_t col_num;
    enum cell_types type;
    lxw_format *format;

    union {
        double number;
        int32_t string_id;
        char *formula;
    } u;

    union {
        double number;
        char *string;
    } formula_result;

    /* List pointers for queue.h. */
    TAILQ_ENTRY (lxw_cell) list_pointers;
} lxw_cell;

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

/**
 * @brief Write a number to a worksheet cell.
 *
 * @param worksheet Pointer to the lxw_worksheet instance to be updated.
 * @param row       The zero indexed row number.
 * @param col       The zero indexed column number.
 * @param number    The number to write to the cell.
 * @param format    A pointer to a Format instance or NULL.
 *
 * @return A #lxw_write_error code.
 *
 * The `worksheet_write_number()` method writes numeric types to the cell
 * specified by `row` and `column`:
 *
 * @code
 *     worksheet_write_number(worksheet, 0, 0, 123456, NULL);
 *     worksheet_write_number(worksheet, 1, 0, 2.3451, NULL);
 * @endcode
 *
 * @image html write_number01.png
 *
 * The native data type for all numbers in Excel is a IEEE-754 64-bit
 * double-precision floating point, which is also the default type used by
 * `%worksheet_write_number`.
 *
 * The `format` parameter is used to apply formatting to the cell. This
 * parameter can be `NULL` to indicate no formatting or it can be a
 * @ref format.h "Format" object.
 *
 * @code
 *     lxw_format *format = workbook_add_format(workbook);
 *     format_set_num_format(format, "$#,##0.00");
 *
 *     worksheet_write_number(worksheet, 0, 0, 1234.567, format);
 * @endcode
 *
 * @image html write_number02.png
 *
 */
int8_t worksheet_write_number(lxw_worksheet *worksheet,
                              lxw_row_t row,
                              lxw_col_t col, double number,
                              lxw_format *format);
/**
 * @brief Write a string to a worksheet cell.
 *
 * @param worksheet Pointer to the lxw_worksheet instance to be updated.
 * @param row       The zero indexed row number.
 * @param col       The zero indexed column number.
 * @param string    String to write to cell.
 * @param format    A pointer to a Format instance or NULL.
 *
 * @return A #lxw_write_error code.
 *
 * The `%worksheet_write_string()` method writes a string to the cell
 * specified by `row` and `column`:
 *
 * @code
 *     worksheet_write_string(worksheet, 0, 0, "This phrase is English!", NULL);
 * @endcode
 *
 * @image html write_string01.png
 *
 * The `format` parameter is used to apply formatting to the cell. This
 * parameter can be `NULL` to indicate no formatting or it can be a
 * @ref format.h "Format" object:
 *
 * @code
 *     lxw_format *format = workbook_add_format(workbook);
 *     format_set_bold(format);
 *
 *     worksheet_write_string(worksheet, 0, 0, "This phrase is Bold!", format);
 * @endcode
 *
 * @image html write_string02.png
 *
 * Unicode strings are supported in UTF-8 encoding. This generally requires
 * that your source file is UTF-8 encoded or that the data has been read from
 * a UTF-8 source:
 *
 * @code
 *    worksheet_write_string(worksheet, 0, 0, "Это фраза на русском!", NULL);
 * @endcode
 *
 * @image html write_string03.png
 *
 */
int8_t worksheet_write_string(lxw_worksheet *worksheet,
                              lxw_row_t row,
                              lxw_col_t col, const char *string,
                              lxw_format *format);
/**
 * @brief Write a formula to a worksheet cell.
 *
 * @param worksheet Pointer to the lxw_worksheet instance to be updated.
 * @param row       The zero indexed row number.
 * @param col       The zero indexed column number.
 * @param formula   Formula string to write to cell.
 * @param format    A pointer to a Format instance or NULL.
 *
 * @return A #lxw_write_error code.
 *
 * The `%worksheet_write_formula()` method writes a formula or function to the cell
 * specified by `row` and `column`:
 *
 * @code
 *  worksheet_write_formula(worksheet, 0, 0, "=B3 + 6",                    NULL);
 *  worksheet_write_formula(worksheet, 1, 0, "=SIN(PI()/4)",               NULL);
 *  worksheet_write_formula(worksheet, 2, 0, "=SUM(A1:A2)",                NULL);
 *  worksheet_write_formula(worksheet, 3, 0, "=IF(A3>1,\"Yes\", \"No\")",  NULL);
 *  worksheet_write_formula(worksheet, 4, 0, "=AVERAGE(1, 2, 3, 4)",       NULL);
 *  worksheet_write_formula(worksheet, 5, 0, "=DATEVALUE(\"1-Jan-2013\")", NULL);
 * @endcode
 *
 * @image html write_formula01.png
 *
 * The `format` parameter is used to apply formatting to the cell. This
 * parameter can be `NULL` to indicate no formatting or it can be a
 * @ref format.h "Format" object.
 *
 * Libxlsxwriter doesn't calculate the value of a formula and instead stores a
 * default value of `0`. The correct formula result is displayed in Excel, as
 * shown in the example above, since it recalculates the formulas when it loads
 * the file. For cases where this is an issue see the
 * `worksheet_write_formula_num()` function and the discussion in that section.
 *
 * Formulas must be written with the US style separator/range operator which
 * is a comma (not semi-colon). Therefore a formula with multiple values
 * should be written as follows:
 *
 * @code
 *     worksheet_write_formula(worksheet, 0, 0, "=SUM(1, 2, 3)", NULL); // OK.
 *     worksheet_write_formula(worksheet, 1, 0, "=SUM(1; 2; 3)", NULL); // NO. Error on load.
 * @endcode
 *
 */
int8_t worksheet_write_formula(lxw_worksheet *worksheet,
                               lxw_row_t row,
                               lxw_col_t col, const char *formula,
                               lxw_format *format);
/**
 * @brief Write a formula to a worksheet cell with a user defined result.
 *
 * @param worksheet Pointer to the lxw_worksheet instance to be updated.
 * @param row       The zero indexed row number.
 * @param col       The zero indexed column number.
 * @param formula   Formula string to write to cell.
 * @param format    A pointer to a Format instance or NULL.
 * @param result    A user defined result for a formula.
 *
 * @return A #lxw_write_error code.
 *
 * The `%worksheet_write_formula_num()` method writes a formula or function to
 * the cell specified by `row` and `column` with a user defined result:
 *
 * @code
 *     // Required as a workaround only.
 *     worksheet_write_formula_num(worksheet, 0, 0, "=1 + 2", NULL, 3);
 * @endcode
 *
 * Libxlsxwriter doesn't calculate the value of a formula and instead stores
 * the value `0` as the formula result. It then sets a global flag in the XLSX
 * file to say that all formulas and functions should be recalculated when the
 * file is opened.
 *
 * This is the method recommended in the Excel documentation and in general it
 * works fine with spreadsheet applications.
 *
 * However, applications that don't have a facility to calculate formulas,
 * such as Excel Viewer, or some mobile applications will only display the `0`
 * results.
 *
 * If required, the `%worksheet_write_formula_num()` function can be used to
 * specify a formula and its result.
 *
 * This function is rarely required and is only provided for compatibility
 * with some third party applications. For most applications the
 * worksheet_write_formula() function is the recommended way of writing
 * formulas.
 *
 */
int8_t worksheet_write_formula_num(lxw_worksheet *worksheet,
                                   lxw_row_t row,
                                   lxw_col_t col,
                                   const char *formula,
                                   lxw_format *format, double result);

/**
 * @brief Write a date or time to a worksheet cell.
 *
 * @param worksheet Pointer to the lxw_worksheet instance to be updated.
 * @param row       The zero indexed row number.
 * @param col       The zero indexed column number.
 * @param datetime  The datetime to write to the cell.
 * @param format    A pointer to a Format instance or NULL.
 *
 * @return A #lxw_write_error code.
 *
 * The `worksheet_write_datetime()` method can be used to write a date or time
 * to the cell specified by `row` and `column`:
 *
 * @dontinclude dates_and_times02.c
 * @skip include
 * @until num_format
 * @skip Feb
 * @until }
 *
 * The `format` parameter should be used to apply formatting to the cell using
 * a @ref format.h "Format" object as shown above. Without a date format the
 * datetime will appear as a number only.
 *
 * See @ref working_with_dates for more information about handling dates and
 * times in libxlsxwriter.
 */
int8_t worksheet_write_datetime(lxw_worksheet *worksheet,
                                lxw_row_t row,
                                lxw_col_t col, lxw_datetime *datetime,
                                lxw_format *format);

/**
 * @brief Write a formatted blank worksheet cell.
 *
 * @param worksheet Pointer to the lxw_worksheet instance to be updated.
 * @param row       The zero indexed row number.
 * @param col       The zero indexed column number.
 * @param format    A pointer to a Format instance or NULL.
 *
 * @return A #lxw_write_error code.
 *
 * Write a blank cell specified by `row` and `column`:
 *
 * @code
 *     worksheet_write_blank(worksheet, 1, 1, border_format);
 * @endcode
 *
 * This method is used to add formatting to a cell which doesn't contain a
 * string or number value.
 *
 * Excel differentiates between an "Empty" cell and a "Blank" cell. An Empty
 * cell is a cell which doesn't contain data or formatting whilst a Blank cell
 * doesn't contain data but does contain formatting. Excel stores Blank cells
 * but ignores Empty cells.
 *
 * As such, if you write an empty cell without formatting it is ignored.
 *
 */
int8_t worksheet_write_blank(lxw_worksheet *worksheet,
                             lxw_row_t row, lxw_col_t col,
                             lxw_format *format);

/**
 * @brief Set the properties for a row of cells.
 *
 * @param worksheet Pointer to a lxw_worksheet instance to be updated.
 * @param row       The zero indexed row number.
 * @param height    The row height.
 * @param format    A pointer to a Format instance or NULL.
 * @param options   Optional row parameters: hidden, level, collapsed.
 *
 * The `%worksheet_set_row()` method is used to change the default properties
 * of a row. The most common use for this method is to change the height of a
 * row:
 *
 * @code
 *     // Set the height of Row 1 to 20.
 *     worksheet_set_row(worksheet, 0, 20, NULL, NULL);
 * @endcode
 *
 * The other common use for `%worksheet_set_row()` is to set the a @ref
 * format.h "Format" for all cells in the row:
 *
 * @code
 *     lxw_format *bold = workbook_add_format(workbook);
 *     format_set_bold(bold);
 *
 *     // Set the header row to bold.
 *     worksheet_set_row(worksheet, 0, 15, bold, NULL);
 * @endcode
 *
 * If you wish to set the format of a row without changing the height you can
 * pass the default row height of #LXW_DEF_ROW_HEIGHT = 15:
 *
 * @code
 *     worksheet_set_row(worksheet, 0, LXW_DEF_ROW_HEIGHT, format, NULL);
 *     worksheet_set_row(worksheet, 0, 15, format, NULL); // Same as above.
 * @endcode
 *
 * The `format` parameter will be applied to any cells in the row that don't
 * have a format. As with Excel the row format is overridden by an explicit
 * cell format. For example:
 *
 * @code
 *     // Row 1 has format1.
 *     worksheet_set_row(worksheet, 0, 15, format1, NULL);
 *
 *     // Cell A1 in Row 1 defaults to format1.
 *     worksheet_write_string(worksheet, 0, 0, "Hello", NULL);
 *
 *     // Cell B1 in Row 1 keeps format2.
 *     worksheet_write_string(worksheet, 0, 1, "Hello", format2);
 * @endcode
 *
 * The `options` parameter is a #lxw_row_col_options struct. It has the
 * following members but currently only the `hidden` property is supported:
 *
 * * `hidden`
 * * `level`
 * * `collapsed`
 *
 * The `"hidden"` option is used to hide a row. This can be used, for example,
 * to hide intermediary steps in a complicated calculation:
 *
 * @code
 *     lxw_row_col_options options = {.hidden = 1, .level = 0, .collapsed = 0};
 *
 *     // Hide the fourth row.
 *     worksheet_set_row(worksheet, 3, 20, NULL, &options);
 * @endcode
 *
 */
int8_t worksheet_set_row(lxw_worksheet *worksheet,
                         lxw_row_t row,
                         double height,
                         lxw_format *format, lxw_row_col_options *options);

/**
 * @brief Set the properties for one or more columns of cells.
 *
 * @param worksheet Pointer to a lxw_worksheet instance to be updated.
 * @param first_col The zero indexed first column.
 * @param last_col  The zero indexed last column.
 * @param width     The width of the column(s).
 * @param format    A pointer to a Format instance or NULL.
 * @param options   Optional row parameters: hidden, level, collapsed.
 *
 * The `%worksheet_set_column()` method can be used to change the default
 * properties of a single column or a range of columns:
 *
 * @code
 *     // Width of columns B:D set to 30.
 *     worksheet_set_column(worksheet, 1, 3, 30, NULL, NULL);
 *
 * @endcode
 *
 * If `%worksheet_set_column()` is applied to a single column the value of
 * `first_col` and `last_col` should be the same:
 *
 * @code
 *     // Width of column B set to 30.
 *     worksheet_set_column(worksheet, 1, 1, 30, NULL, NULL);
 *
 * @endcode
 *
 * It is also possible, and generally clearer, to specify a column range using the
 * form of `COLS()` macro:
 *
 * @code
 *     worksheet_set_column(worksheet, 4, 4, 20, NULL, NULL);
 *     worksheet_set_column(worksheet, 5, 8, 30, NULL, NULL);
 *
 *     // Same as the examples above but clearer.
 *     worksheet_set_column(worksheet, COLS("E:E"), 20, NULL, NULL);
 *     worksheet_set_column(worksheet, COLS("F:H"), 30, NULL, NULL);
 *
 * @endcode
 *
 * The width corresponds to the column width value that is specified in
 * Excel. It is approximately equal to the length of a string in the default
 * font of Calibri 11. Unfortunately, there is no way to specify "AutoFit" for
 * a column in the Excel file format. This feature is only available at
 * runtime from within Excel. It is possible to simulate "AutoFit" by tracking
 * the width of the data in the column as your write it.
 *
 * As usual the @ref format.h `format` parameter is optional. If you wish to
 * set the format without changing the width you can pass default col width of
 * #LXW_DEF_COL_WIDTH = 8.43:
 *
 * @code
 *     lxw_format *bold = workbook_add_format(workbook);
 *     format_set_bold(bold);
 *
 *     // Set the first column to bold.
 *     worksheet_set_column(worksheet, 0, 0, LXW_DEF_COL_HEIGHT, bold, NULL);
 * @endcode
 *
 * The `format` parameter will be applied to any cells in the column that
 * don't have a format. For example:
 *
 * @code
 *     // Column 1 has format1.
 *     worksheet_set_column(worksheet, COLS("A:A"), 8.43, format1, NULL);
 *
 *     // Cell A1 in column 1 defaults to format1.
 *     worksheet_write_string(worksheet, 0, 0, "Hello", NULL);
 *
 *     // Cell A2 in column 1 keeps format2.
 *     worksheet_write_string(worksheet, 1, 0, "Hello", format2);
 * @endcode
 *
 * As in Excel a row format takes precedence over a default column format:
 *
 * @code
 *     // Row 1 has format1.
 *     worksheet_set_row(worksheet, 0, 15, format1, NULL);
 *
 *     // Col 1 has format2.
 *     worksheet_set_column(worksheet, COLS("A:A"), 8.43, format2, NULL);
 *
 *     // Cell A1 defaults to format1, the row format.
 *     worksheet_write_string(worksheet, 0, 0, "Hello", NULL);
 *
 *    // Cell A2 keeps format2, the column format.
 *     worksheet_write_string(worksheet, 1, 0, "Hello", NULL);
 * @endcode
 *
 * The `options` parameter is a #lxw_row_col_options struct. It has the
 * following members but currently only the `hidden` property is supported:
 *
 * * `hidden`
 * * `level`
 * * `collapsed`
 *
 * The `"hidden"` option is used to hide a column. This can be used, for example,
 * to hide intermediary steps in a complicated calculation:
 *
 * @code
 *     lxw_row_col_options options = {.hidden = 1, .level = 0, .collapsed = 0};
 *
 *     worksheet_set_column(worksheet, COLS("A:A"), 8.43, NULL, &options);
 * @endcode
 *
 */
int8_t worksheet_set_column(lxw_worksheet *worksheet, lxw_col_t first_col,
                            lxw_col_t last_col, double width,
                            lxw_format *format, lxw_row_col_options *options);


lxw_worksheet *_new_worksheet(lxw_worksheet_init_data *init_data);
void _free_worksheet(lxw_worksheet *worksheet);
void _worksheet_assemble_xml_file(lxw_worksheet *worksheet);

/* Declarations required for unit testing. */
#ifdef TESTING

STATIC void _worksheet_xml_declaration(lxw_worksheet *worksheet);
STATIC void _worksheet_write_worksheet(lxw_worksheet *worksheet);
STATIC void _worksheet_write_dimension(lxw_worksheet *worksheet);
STATIC void _worksheet_write_sheet_view(lxw_worksheet *worksheet);
STATIC void _worksheet_write_sheet_views(lxw_worksheet *worksheet);
STATIC void _worksheet_write_sheet_format_pr(lxw_worksheet *worksheet);
STATIC void _worksheet_write_sheet_data(lxw_worksheet *worksheet);
STATIC void _worksheet_write_page_margins(lxw_worksheet *worksheet);
STATIC void _write_row(lxw_worksheet *worksheet, lxw_row *row, char *spans);
STATIC void _write_col_info(lxw_worksheet *worksheet,
                            lxw_col_options *options);
STATIC lxw_row *_get_row(struct lxw_table_rows *table, lxw_row_t row_num);

#endif /* TESTING */

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */

#endif /* __LXW_WORKSHEET_H__ */
