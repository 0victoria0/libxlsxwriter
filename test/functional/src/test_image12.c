/*****************************************************************************
 * Test cases for libxlsxwriter.
 *
 * Test to compare output against Excel files.
 *
 * Copyright 2014-2016, John McNamara, jmcnamara@cpan.org
 *
 */

#include "xlsxwriter.h"

int main() {

    lxw_workbook  *workbook  = new_workbook("test_image12.xlsx");
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);

    worksheet_set_row(worksheet, 1, 75, NULL, NULL);
    worksheet_set_column(worksheet, 2, 2, 32, NULL, NULL);

    worksheet_insert_image(worksheet, CELL("C2"), "images/logo.png");

    return workbook_close(workbook);
}
