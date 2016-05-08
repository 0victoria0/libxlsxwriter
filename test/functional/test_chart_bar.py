###############################################################################
#
# Tests for libxlsxwriter.
#
# Copyright 2014-2016, John McNamara, jmcnamara@cpan.org
#

import base_test_class

class TestCompareXLSXFiles(base_test_class.XLSXBaseTest):
    """
    Test file created with libxlsxwriter against a file created by Excel.

    """

    def test_chart_bar01(self):
        self.run_exe_test('test_chart_bar01')

    def test_chart_bar02(self):
        self.run_exe_test('test_chart_bar02')

    def test_chart_bar03(self):
        self.run_exe_test('test_chart_bar03')

    def test_chart_bar04(self):
        self.run_exe_test('test_chart_bar04')

    def test_chart_bar05(self):
        self.run_exe_test('test_chart_bar05')

    def test_chart_bar08(self):
        self.run_exe_test('test_chart_bar08')

    def test_chart_bar09(self):
        self.run_exe_test('test_chart_bar09')

    def test_chart_bar10(self):
        self.run_exe_test('test_chart_bar10')

    def test_chart_bar11(self):
        self.run_exe_test('test_chart_bar11')

    def test_chart_bar51(self):
        self.run_exe_test('test_chart_bar51')

    def test_chart_bar52(self):
        self.run_exe_test('test_chart_bar52', 'chart_bar02.xlsx')

    def test_chart_bar53(self):
        self.run_exe_test('test_chart_bar53', 'chart_bar03.xlsx')

    def test_chart_bar54(self):
        self.run_exe_test('test_chart_bar54', 'chart_bar04.xlsx')

    def test_chart_bar55(self):
        self.run_exe_test('test_chart_bar55', 'chart_bar05.xlsx')
