/*****************************************************************************
 * chart - A library for creating Excel XLSX chart files.
 *
 * Used in conjunction with the libxlsxwriter library.
 *
 * Copyright 2014-2016, John McNamara, jmcnamara@cpan.org. See LICENSE.txt.
 *
 */

#include "xlsxwriter/xmlwriter.h"
#include "xlsxwriter/chart.h"
#include "xlsxwriter/utility.h"

/*
 * Forward declarations.
 */

/*****************************************************************************
 *
 * Private functions.
 *
 ****************************************************************************/

/*
 * Create a new chart object.
 */
lxw_chart *
lxw_chart_new(uint8_t type)
{
    lxw_chart *chart = calloc(1, sizeof(lxw_chart));
    GOTO_LABEL_ON_MEM_ERROR(chart, mem_error);

    chart->series_list = calloc(1, sizeof(struct lxw_chart_series_list));
    GOTO_LABEL_ON_MEM_ERROR(chart->series_list, mem_error);
    STAILQ_INIT(chart->series_list);

    chart->type = type;

    /* Set the default axis positions. */
    strcpy(chart->cat_axis_position, "b");
    strcpy(chart->val_axis_position, "l");

    /* Set the default grouping. */
    strcpy(chart->grouping, "clustered");

    strcpy(chart->cross_between, "between");

    strcpy(chart->x_axis.default_num_format, "General");
    strcpy(chart->y_axis.default_num_format, "General");

    chart->series_overlap_1 = 100;

    return chart;

mem_error:
    lxw_chart_free(chart);
    return NULL;
}

/*
 * Free a series range object.
 */
void
lxw_chart_free_range(lxw_series_range *range)
{
    struct lxw_series_data_point *data_point;

    if (!range)
        return;

    while (!STAILQ_EMPTY(range->data_cache)) {
        data_point = STAILQ_FIRST(range->data_cache);
        STAILQ_REMOVE_HEAD(range->data_cache, list_pointers);

        free(data_point);
    }

    free(range->data_cache);
    free(range->formula);
    free(range->sheetname);
    free(range);
}

/*
 * Free a series object.
 */
void
lxw_chart_series_free(lxw_chart_series *series)
{
    if (!series)
        return;

    lxw_chart_free_range(series->categories);
    lxw_chart_free_range(series->values);

    free(series);
}

/*
 * Free a chart object.
 */
void
lxw_chart_free(lxw_chart *chart)
{
    lxw_chart_series *series;

    if (!chart)
        return;

    if (chart->series_list) {
        while (!STAILQ_EMPTY(chart->series_list)) {
            series = STAILQ_FIRST(chart->series_list);
            STAILQ_REMOVE_HEAD(chart->series_list, list_pointers);

            lxw_chart_series_free(series);
        }

        free(chart->series_list);
    }

    free(chart);
}

/*
 * Add unique ids for primary or secondary axes.
 */
STATIC void
_chart_add_axis_ids(lxw_chart *self)
{
    uint32_t chart_id = 50010000 + self->id;
    uint32_t axis_count = 1;

    self->axis_id_1 = chart_id + axis_count;
    self->axis_id_2 = self->axis_id_1 + 1;
}

/*****************************************************************************
 *
 * XML functions.
 *
 ****************************************************************************/

/*
 * Write the XML declaration.
 */
STATIC void
_chart_xml_declaration(lxw_chart *self)
{
    lxw_xml_declaration(self->file);
}

/*
 * Write the <c:chartSpace> element.
 */
STATIC void
_chart_write_chart_space(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char xmlns_c[] = LXW_SCHEMA_DRAWING "/chart";
    char xmlns_a[] = LXW_SCHEMA_DRAWING "/main";
    char xmlns_r[] = LXW_SCHEMA_OFFICEDOC "/relationships";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("xmlns:c", xmlns_c);
    LXW_PUSH_ATTRIBUTES_STR("xmlns:a", xmlns_a);
    LXW_PUSH_ATTRIBUTES_STR("xmlns:r", xmlns_r);

    lxw_xml_start_tag(self->file, "c:chartSpace", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:lang> element.
 */
STATIC void
_chart_write_lang(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", "en-US");

    lxw_xml_empty_tag(self->file, "c:lang", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:layout> element.
 */
STATIC void
_chart_write_layout(lxw_chart *self)
{
    lxw_xml_empty_tag(self->file, "c:layout", NULL);
}

/*
 * Write the <c:grouping> element.
 */
STATIC void
_chart_write_grouping(lxw_chart *self, char *grouping)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", grouping);

    lxw_xml_empty_tag(self->file, "c:grouping", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:idx> element.
 */
STATIC void
_chart_write_idx(lxw_chart *self, uint16_t index)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_INT("val", index);

    lxw_xml_empty_tag(self->file, "c:idx", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:order> element.
 */
STATIC void
_chart_write_order(lxw_chart *self, uint16_t index)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_INT("val", index);

    lxw_xml_empty_tag(self->file, "c:order", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:axId> element.
 */
STATIC void
_chart_write_axis_id(lxw_chart *self, uint32_t axis_id)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_INT("val", axis_id);

    lxw_xml_empty_tag(self->file, "c:axId", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:axId> element.
 */
STATIC void
_chart_write_axis_ids(lxw_chart *self)
{
    if (!self->axis_id_1)
        _chart_add_axis_ids(self);

    _chart_write_axis_id(self, self->axis_id_1);
    _chart_write_axis_id(self, self->axis_id_2);
}

/*
 * Write the <c:ptCount> element.
 */
STATIC void
_chart_write_pt_count(lxw_chart *self, uint16_t num_data_points)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_INT("val", num_data_points);

    lxw_xml_empty_tag(self->file, "c:ptCount", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:v> element.
 */
STATIC void
_chart_write_v(lxw_chart *self, double number)
{
    char data[LXW_ATTR_32];

    lxw_snprintf(data, LXW_ATTR_32, "%.16g", number);

    lxw_xml_data_element(self->file, "c:v", data, NULL);
}

/*
 * Write the <c:pt> element.
 */
STATIC void
_chart_write_pt(lxw_chart *self, uint16_t index, double number)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_INT("idx", index);

    lxw_xml_start_tag(self->file, "c:pt", &attributes);

    /* Write the c:v element. */
    _chart_write_v(self, number);

    lxw_xml_end_tag(self->file, "c:pt");

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:formatCode> element.
 */
STATIC void
_chart_write_format_code(lxw_chart *self)
{
    lxw_xml_data_element(self->file, "c:formatCode", "General", NULL);
}

/*
 * Write the <c:numCache> element.
 */
STATIC void
_chart_write_num_cache(lxw_chart *self, lxw_series_range *range)
{
    lxw_series_data_point *data_point;
    uint16_t index = 0;

    lxw_xml_start_tag(self->file, "c:numCache", NULL);

    /* Write the c:formatCode element. */
    _chart_write_format_code(self);

    /* Write the c:ptCount element. */
    _chart_write_pt_count(self, range->num_data_points);

    STAILQ_FOREACH(data_point, range->data_cache, list_pointers) {
        /* Write the c:pt element. */
        _chart_write_pt(self, index, data_point->number);
        index++;
    }

    lxw_xml_end_tag(self->file, "c:numCache");
}

/*
 * Write the cached data elements.
 */
STATIC void
_chart_write_data_cache(lxw_chart *self, lxw_series_range *range)
{
    /* Only number cache is supported, for now. TODO: add string support. */
    /* Write the c:numCache element. */
    _chart_write_num_cache(self, range);
}

/*
 * Write the <c:f> element.
 */
STATIC void
_chart_write_f(lxw_chart *self, char *formula)
{
    lxw_xml_data_element(self->file, "c:f", formula, NULL);
}

/*
 * Write the <c:numRef> element.
 */
STATIC void
_chart_write_num_ref(lxw_chart *self, lxw_series_range *range)
{
    lxw_xml_start_tag(self->file, "c:numRef", NULL);

    /* Write the c:f element. */
    _chart_write_f(self, range->formula);

    if (!STAILQ_EMPTY(range->data_cache)) {
        /* Write the c:numCache element. */
        _chart_write_data_cache(self, range);
    }

    lxw_xml_end_tag(self->file, "c:numRef");
}

/*
 * Write the <c:symbol> element.
 */
STATIC void
_chart_write_symbol(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char val[] = "none";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", val);

    lxw_xml_empty_tag(self->file, "c:symbol", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:marker> element.
 */
STATIC void
_chart_write_marker(lxw_chart *self)
{
    if (!self->has_markers)
        return;

    lxw_xml_start_tag(self->file, "c:marker", NULL);

    /* Write the c:symbol element. */
    _chart_write_symbol(self);

    lxw_xml_end_tag(self->file, "c:marker");
}

/*
 * Write the <c:marker> element.
 */
STATIC void
_chart_write_marker_value(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char val[] = "1";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", val);

    lxw_xml_empty_tag(self->file, "c:marker", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:cat> element.
 */
STATIC void
_chart_write_cat(lxw_chart *self, lxw_chart_series *series)
{
    /* Ignore <c:cat> elements for charts without category values. */
    if (!series->categories->formula)
        return;

    /* TODO */
    self->cat_has_num_fmt = LXW_TRUE;

    lxw_xml_start_tag(self->file, "c:cat", NULL);

    /* Write the c:numRef element. */
    _chart_write_num_ref(self, series->categories);

    lxw_xml_end_tag(self->file, "c:cat");
}

/*
 * Write the <c:val> element.
 */
STATIC void
_chart_write_val(lxw_chart *self, lxw_chart_series *series)
{
    lxw_xml_start_tag(self->file, "c:val", NULL);

    /* Write the c:numRef element. */
    _chart_write_num_ref(self, series->values);

    lxw_xml_end_tag(self->file, "c:val");
}

/*
 * Write the <c:ser> element.
 */
STATIC void
_chart_write_ser(lxw_chart *self, lxw_chart_series *series)
{
    uint16_t index = self->series_index++;

    lxw_xml_start_tag(self->file, "c:ser", NULL);

    /* Write the c:idx element. */
    _chart_write_idx(self, index);

    /* Write the c:order element. */
    _chart_write_order(self, index);

    /* Write the c:marker element. */
    _chart_write_marker(self);

    /* Write the c:cat element. */
    _chart_write_cat(self, series);

    /* Write the c:val element. */
    _chart_write_val(self, series);

    lxw_xml_end_tag(self->file, "c:ser");
}

/*
 * Write the <c:orientation> element.
 */
STATIC void
_chart_write_orientation(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char val[] = "minMax";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", val);

    lxw_xml_empty_tag(self->file, "c:orientation", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:scaling> element.
 */
STATIC void
_chart_write_scaling(lxw_chart *self)
{
    lxw_xml_start_tag(self->file, "c:scaling", NULL);

    /* Write the c:orientation element. */
    _chart_write_orientation(self);

    lxw_xml_end_tag(self->file, "c:scaling");
}

/*
 * Write the <c:axPos> element.
 */
STATIC void
_chart_write_axis_pos(lxw_chart *self, char *position)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", position);

    lxw_xml_empty_tag(self->file, "c:axPos", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:tickLblPos> element.
 */
STATIC void
_chart_write_tick_lbl_pos(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char val[] = "nextTo";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", val);

    lxw_xml_empty_tag(self->file, "c:tickLblPos", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:crossAx> element.
 */
STATIC void
_chart_write_cross_axis(lxw_chart *self, uint32_t axis_id)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_INT("val", axis_id);

    lxw_xml_empty_tag(self->file, "c:crossAx", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:crosses> element.
 */
STATIC void
_chart_write_crosses(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char val[] = "autoZero";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", val);

    lxw_xml_empty_tag(self->file, "c:crosses", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:auto> element.
 */
STATIC void
_chart_write_auto(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char val[] = "1";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", val);

    lxw_xml_empty_tag(self->file, "c:auto", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:lblAlgn> element.
 */
STATIC void
_chart_write_lbl_algn(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char val[] = "ctr";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", val);

    lxw_xml_empty_tag(self->file, "c:lblAlgn", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:lblOffset> element.
 */
STATIC void
_chart_write_lbl_offset(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char val[] = "100";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", val);

    lxw_xml_empty_tag(self->file, "c:lblOffset", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:majorGridlines> element.
 */
STATIC void
_chart_write_major_gridlines(lxw_chart *self)
{
    lxw_xml_empty_tag(self->file, "c:majorGridlines", NULL);
}

/*
 * Write the <c:numFmt> element.
 */
STATIC void
_chart_write_number_format(lxw_chart *self, lxw_axis *axis)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char source_linked[] = "1";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("formatCode", axis->default_num_format);
    LXW_PUSH_ATTRIBUTES_STR("sourceLinked", source_linked);

    lxw_xml_empty_tag(self->file, "c:numFmt", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:crossBetween> element.
 */
STATIC void
_chart_write_cross_between(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", self->cross_between);

    lxw_xml_empty_tag(self->file, "c:crossBetween", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:legendPos> element.
 */
STATIC void
_chart_write_legend_pos(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char val[] = "r";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", val);

    lxw_xml_empty_tag(self->file, "c:legendPos", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:legend> element.
 */
STATIC void
_chart_write_legend(lxw_chart *self)
{
    lxw_xml_start_tag(self->file, "c:legend", NULL);

    /* Write the c:legendPos element. */
    _chart_write_legend_pos(self);

    /* Write the c:layout element. */
    _chart_write_layout(self);

    lxw_xml_end_tag(self->file, "c:legend");
}

/*
 * Write the <c:plotVisOnly> element.
 */
STATIC void
_chart_write_plot_vis_only(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char val[] = "1";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", val);

    lxw_xml_empty_tag(self->file, "c:plotVisOnly", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:headerFooter> element.
 */
STATIC void
_chart_write_header_footer(lxw_chart *self)
{
    lxw_xml_empty_tag(self->file, "c:headerFooter", NULL);
}

/*
 * Write the <c:pageMargins> element.
 */
STATIC void
_chart_write_page_margins(lxw_chart *self)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;
    char b[] = "0.75";
    char l[] = "0.7";
    char r[] = "0.7";
    char t[] = "0.75";
    char header[] = "0.3";
    char footer[] = "0.3";

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("b", b);
    LXW_PUSH_ATTRIBUTES_STR("l", l);
    LXW_PUSH_ATTRIBUTES_STR("r", r);
    LXW_PUSH_ATTRIBUTES_STR("t", t);
    LXW_PUSH_ATTRIBUTES_STR("header", header);
    LXW_PUSH_ATTRIBUTES_STR("footer", footer);

    lxw_xml_empty_tag(self->file, "c:pageMargins", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:pageSetup> element.
 */
STATIC void
_chart_write_page_setup(lxw_chart *self)
{
    lxw_xml_empty_tag(self->file, "c:pageSetup", NULL);
}

/*
 * Write the <c:printSettings> element.
 */
STATIC void
_chart_write_print_settings(lxw_chart *self)
{
    lxw_xml_start_tag(self->file, "c:printSettings", NULL);

    /* Write the c:headerFooter element. */
    _chart_write_header_footer(self);

    /* Write the c:pageMargins element. */
    _chart_write_page_margins(self);

    /* Write the c:pageSetup element. */
    _chart_write_page_setup(self);

    lxw_xml_end_tag(self->file, "c:printSettings");
}

/*
 * Write the <c:overlap> element.
 */
STATIC void
_chart_write_overlap(lxw_chart *self, int overlap)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_INT("val", overlap);

    lxw_xml_empty_tag(self->file, "c:overlap", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write the <c:catAx> element. Usually the X axis.
 */
STATIC void
_chart_write_cat_axis(lxw_chart *self)
{
    char *position = self->cat_axis_position;

    lxw_xml_start_tag(self->file, "c:catAx", NULL);

    _chart_write_axis_id(self, self->axis_id_1);

    /* Write the c:scaling element. */
    _chart_write_scaling(self);

    /* Write the c:axPos element. */
    _chart_write_axis_pos(self, position);

    /* Write the c:numFmt element. */
    if (self->cat_has_num_fmt)
        _chart_write_number_format(self, &self->x_axis);

    /* Write the c:tickLblPos element. */
    _chart_write_tick_lbl_pos(self);

    /* Write the c:crossAx element. */
    _chart_write_cross_axis(self, self->axis_id_2);

    /* Write the c:crosses element. */
    _chart_write_crosses(self);

    /* Write the c:auto element. */
    _chart_write_auto(self);

    /* Write the c:lblAlgn element. */
    _chart_write_lbl_algn(self);

    /* Write the c:lblOffset element. */
    _chart_write_lbl_offset(self);

    lxw_xml_end_tag(self->file, "c:catAx");
}

/*
 * Write the <c:valAx> element.
 */
STATIC void
_chart_write_val_axis(lxw_chart *self)
{
    char *position = self->val_axis_position;

    lxw_xml_start_tag(self->file, "c:valAx", NULL);

    _chart_write_axis_id(self, self->axis_id_2);

    /* Write the c:scaling element. */
    _chart_write_scaling(self);

    /* Write the c:axPos element. */
    _chart_write_axis_pos(self, position);

    /* Write the c:majorGridlines element. */
    _chart_write_major_gridlines(self);

    /* Write the c:numFmt element. */
    _chart_write_number_format(self, &self->y_axis);

    /* Write the c:tickLblPos element. */
    _chart_write_tick_lbl_pos(self);

    /* Write the c:crossAx element. */
    _chart_write_cross_axis(self, self->axis_id_1);

    /* Write the c:crosses element. */
    _chart_write_crosses(self);

    /* Write the c:crossBetween element. */
    _chart_write_cross_between(self);

    lxw_xml_end_tag(self->file, "c:valAx");
}

/*****************************************************************************
 * Bar chart functions.
 */

/*
 * Write the <c:barDir> element.
 */
STATIC void
_chart_write_bar_dir(lxw_chart *self, char *type)
{
    struct xml_attribute_list attributes;
    struct xml_attribute *attribute;

    LXW_INIT_ATTRIBUTES();
    LXW_PUSH_ATTRIBUTES_STR("val", type);

    lxw_xml_empty_tag(self->file, "c:barDir", &attributes);

    LXW_FREE_ATTRIBUTES();
}

/*
 * Write a area chart.
 */
STATIC void
_chart_write_area_chart(lxw_chart *self, uint8_t type)
{
    lxw_chart_series *series;

    strcpy(self->grouping, "standard");
    strcpy(self->cross_between, "midCat");

    if (type == LXW_CHART_AREA_STACKED) {
        strcpy(self->grouping, "stacked");
        self->subtype = LXW_CHART_SUBTYPE_STACKED;
    }

    if (type == LXW_CHART_AREA_STACKED_PERCENT) {
        strcpy(self->grouping, "percentStacked");
        strcpy((&self->y_axis)->default_num_format, "0%");
        self->subtype = LXW_CHART_SUBTYPE_STACKED;
    }

    lxw_xml_start_tag(self->file, "c:areaChart", NULL);

    /* Write the c:grouping element. */
    _chart_write_grouping(self, self->grouping);

    STAILQ_FOREACH(series, self->series_list, list_pointers) {
        /* Write the c:ser element. */
        _chart_write_ser(self, series);
    }

    if (self->has_overlap) {
        /* Write the c:overlap element. */
        _chart_write_overlap(self, self->series_overlap_1);
    }

    /* Write the c:axId elements. */
    _chart_write_axis_ids(self);

    lxw_xml_end_tag(self->file, "c:areaChart");
}

/*
 * Write a bar chart.
 */
STATIC void
_chart_write_bar_chart(lxw_chart *self, uint8_t type)
{
    lxw_chart_series *series;

    if (type == LXW_CHART_BAR_STACKED) {
        strcpy(self->grouping, "stacked");
        self->has_overlap = LXW_TRUE;
        self->subtype = LXW_CHART_SUBTYPE_STACKED;
    }

    if (type == LXW_CHART_BAR_STACKED_PERCENT) {
        strcpy(self->grouping, "percentStacked");
        strcpy((&self->y_axis)->default_num_format, "0%");
        self->has_overlap = LXW_TRUE;
        self->subtype = LXW_CHART_SUBTYPE_STACKED;
    }

    /* Override the default axis positions for a bar chart. */
    strcpy(self->cat_axis_position, "l");
    strcpy(self->val_axis_position, "b");

    lxw_xml_start_tag(self->file, "c:barChart", NULL);

    /* Write the c:barDir element. */
    _chart_write_bar_dir(self, "bar");

    /* Write the c:grouping element. */
    _chart_write_grouping(self, self->grouping);

    STAILQ_FOREACH(series, self->series_list, list_pointers) {
        /* Write the c:ser element. */
        _chart_write_ser(self, series);
    }

    if (self->has_overlap) {
        /* Write the c:overlap element. */
        _chart_write_overlap(self, self->series_overlap_1);
    }

    /* Write the c:axId elements. */
    _chart_write_axis_ids(self);

    lxw_xml_end_tag(self->file, "c:barChart");
}

/*
 * Write a column chart.
 */
STATIC void
_chart_write_column_chart(lxw_chart *self, uint8_t type)
{
    lxw_chart_series *series;

    if (type == LXW_CHART_COLUMN_STACKED) {
        strcpy(self->grouping, "stacked");
        self->has_overlap = LXW_TRUE;
        self->subtype = LXW_CHART_SUBTYPE_STACKED;
    }

    if (type == LXW_CHART_COLUMN_STACKED_PERCENT) {
        strcpy(self->grouping, "percentStacked");
        strcpy((&self->y_axis)->default_num_format, "0%");
        self->has_overlap = LXW_TRUE;
        self->subtype = LXW_CHART_SUBTYPE_STACKED;
    }

    lxw_xml_start_tag(self->file, "c:barChart", NULL);

    /* Write the c:barDir element. */
    _chart_write_bar_dir(self, "col");

    /* Write the c:grouping element. */
    _chart_write_grouping(self, self->grouping);

    STAILQ_FOREACH(series, self->series_list, list_pointers) {
        /* Write the c:ser element. */
        _chart_write_ser(self, series);
    }

    if (self->has_overlap) {
        /* Write the c:overlap element. */
        _chart_write_overlap(self, self->series_overlap_1);
    }

    /* Write the c:axId elements. */
    _chart_write_axis_ids(self);

    lxw_xml_end_tag(self->file, "c:barChart");
}

/*
 * Write a line chart.
 */
STATIC void
_chart_write_line_chart(lxw_chart *self)
{
    lxw_chart_series *series;

    self->has_markers = LXW_TRUE;
    strcpy(self->grouping, "standard");

    lxw_xml_start_tag(self->file, "c:lineChart", NULL);

    /* Write the c:grouping element. */
    _chart_write_grouping(self, self->grouping);

    STAILQ_FOREACH(series, self->series_list, list_pointers) {
        /* Write the c:ser element. */
        _chart_write_ser(self, series);
    }

    /* Write the c:marker element. */
    _chart_write_marker_value(self);

    /* Write the c:axId elements. */
    _chart_write_axis_ids(self);

    lxw_xml_end_tag(self->file, "c:lineChart");
}

/*****************************************************************************
 * End of sub chart functions.
 */

/*
 * Write the chart type element.
 */
STATIC void
_chart_write_chart_type(lxw_chart *self, uint8_t type)
{
    switch (type) {

        case LXW_CHART_AREA:
        case LXW_CHART_AREA_STACKED:
        case LXW_CHART_AREA_STACKED_PERCENT:
            _chart_write_area_chart(self, type);
            break;

        case LXW_CHART_BAR:
        case LXW_CHART_BAR_STACKED:
        case LXW_CHART_BAR_STACKED_PERCENT:
            _chart_write_bar_chart(self, type);
            break;

        case LXW_CHART_COLUMN:
        case LXW_CHART_COLUMN_STACKED:
        case LXW_CHART_COLUMN_STACKED_PERCENT:
            _chart_write_column_chart(self, type);
            break;

        case LXW_CHART_LINE:
            _chart_write_line_chart(self);
            break;

        default:
            LXW_WARN_FORMAT("workbook_add_chart(): "
                            "unhandled chart type '%d'", type);
    }

}

/*
 * Write the <c:plotArea> element.
 */
STATIC void
_chart_write_plot_area(lxw_chart *self)
{
    lxw_xml_start_tag(self->file, "c:plotArea", NULL);

    /* Write the c:layout element. */
    _chart_write_layout(self);

    /* Write the subclass chart type elements for primary and secondary axes. */
    _chart_write_chart_type(self, self->type);

}

/*
 * Write the <c:chart> element.
 */
STATIC void
_chart_write_chart(lxw_chart *self)
{
    lxw_xml_start_tag(self->file, "c:chart", NULL);

    /* Write the c:plotArea element. */
    _chart_write_plot_area(self);

    /* Write the c:catAx element. */
    _chart_write_cat_axis(self);

    /* Write the c:valAx element. */
    _chart_write_val_axis(self);

    lxw_xml_end_tag(self->file, "c:plotArea");

    /* Write the c:legend element. */
    _chart_write_legend(self);

    /* Write the c:plotVisOnly element. */
    _chart_write_plot_vis_only(self);

    lxw_xml_end_tag(self->file, "c:chart");
}

/*
 * Assemble and write the XML file.
 */
void
lxw_chart_assemble_xml_file(lxw_chart *self)
{
    /* Write the XML declaration. */
    _chart_xml_declaration(self);

    /* Write the c:chartSpace element. */
    _chart_write_chart_space(self);

    /* Write the c:lang element. */
    _chart_write_lang(self);

    /* Write the c:chart element. */
    _chart_write_chart(self);

    /* Write the c:printSettings element. */
    _chart_write_print_settings(self);

    lxw_xml_end_tag(self->file, "c:chartSpace");
}

/*****************************************************************************
 *
 * Public functions.
 *
 ****************************************************************************/

/*
 * Add data to a data cache in a range object, for testing only.
 */
int
lxw_chart_add_data_cache(lxw_series_range *range, uint8_t *data,
                         uint16_t rows, uint8_t cols, uint8_t col)
{
    struct lxw_series_data_point *data_point;
    uint16_t i;

    range->ignore_cache = LXW_TRUE;
    range->num_data_points = rows;

    /* Initialize the series range data cache. */
    for (i = 0; i < rows; i++) {
        data_point = calloc(1, sizeof(struct lxw_series_data_point));
        STAILQ_INSERT_TAIL(range->data_cache, data_point, list_pointers);
        data_point->number = data[i * cols + col];
    }

    return 0;
}

/*
 * Initialize the data cache in a range object.
 */
int
lxw_chart_init_data_cache(lxw_series_range *range)
{
    /* Initialize the series range data cache. */
    range->data_cache = calloc(1, sizeof(struct lxw_series_data_points));
    RETURN_ON_MEM_ERROR(range->data_cache, -1);
    STAILQ_INIT(range->data_cache);

    return 0;
}

/*
 * Insert an image into the worksheet.
 */
lxw_chart_series *
chart_add_series(lxw_chart *self, char *categories, char *values)
{
    lxw_chart_series *series;

    /* Create a new object to hold the series. */
    series = calloc(1, sizeof(lxw_chart_series));
    GOTO_LABEL_ON_MEM_ERROR(series, mem_error);

    series->categories = calloc(1, sizeof(lxw_series_range));
    GOTO_LABEL_ON_MEM_ERROR(series->categories, mem_error);

    series->values = calloc(1, sizeof(lxw_series_range));
    GOTO_LABEL_ON_MEM_ERROR(series->values, mem_error);

    if (categories) {
        if (categories[0] == '=')
            series->categories->formula = lxw_strdup(categories + 1);
        else
            series->categories->formula = lxw_strdup(categories);
    }

    if (values) {
        if (values[0] == '=')
            series->values->formula = lxw_strdup(values + 1);
        else
            series->values->formula = lxw_strdup(values);
    }

    lxw_chart_init_data_cache(series->categories);
    lxw_chart_init_data_cache(series->values);

    STAILQ_INSERT_TAIL(self->series_list, series, list_pointers);

    return series;

mem_error:
    lxw_chart_series_free(series);
    return NULL;
}
