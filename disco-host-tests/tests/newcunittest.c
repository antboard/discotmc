/*
 * File:   newcunittest.c
 * Author: karlp
 *
 * Created on Jan 28, 2014, 9:17:22 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CUnit/Basic.h>
#include "scpi/scpi.h"
/*
 * CUnit Test Suite
 */
#define SCPI_INPUT_BUFFER_LENGTH 256
static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];

char obuf[1024];
int obuf_idx;
char numbuf[100]; // for printing numbers with units

size_t scpi_impl_write(scpi_t * context, const char * data, size_t len);
int scpi_impl_error(scpi_t * context, int_fast16_t err);


/////// TEST IMPLS 

static int dscpi_output_inner(scpi_t *context, int output)
{
	bool action;
	if (!SCPI_ParamBool(context, &action, true)) {
		return SCPI_RES_ERR;
	}
	printf("turning output %d %s", output + 1, action ? "on" : "off");
	if (output == 0) {
		SCPI_ResultString(context, "output1");
	}
	if (output == 1) {
		SCPI_ResultString(context, "output2");
	}
	if (action) {
		SCPI_ResultString(context, "on");
	} else {
		SCPI_ResultString(context, "off");
	}
	return SCPI_RES_OK;
}

int dscpi_output1(scpi_t *context)
{
	return dscpi_output_inner(context, 0);
}

int dscpi_output2(scpi_t *context)
{
	return dscpi_output_inner(context, 1);
}

int dscpi_output1Q(scpi_t *context)
{
	printf("Q1\n");
	SCPI_ResultString(context, "output1query");
	return SCPI_RES_OK;
}

int dscpi_output2Q(scpi_t *context)
{
	printf("Q2\n");
	SCPI_ResultString(context, "output2query");
	return SCPI_RES_OK;
}

int dscpi_output_load_inner(scpi_t *context, int output)
{
	scpi_number_t load;
	if (!SCPI_ParamNumber(context, &load, true)) {
		return SCPI_RES_ERR;
	}
	/* gw instek actually only allows a choice, but I haven't tried using number params */
	if (output) {
		SCPI_ResultString(context, "output2_load");
	} else {
		SCPI_ResultString(context, "output1_load");
	}
	if (load.type == SCPI_NUM_INF) {
		SCPI_ResultString(context, "infinite");
	} else if (load.type == SCPI_NUM_DEF) {
		SCPI_ResultString(context, "default");
	} else if (load.type == SCPI_NUM_NUMBER) {
		if (load.unit == SCPI_UNIT_NONE) {
			load.unit = SCPI_UNIT_OHM;
		}
		SCPI_NumberToStr(context, &load, numbuf, sizeof (numbuf));
		printf("setting load %d to %s\n", output + 1, numbuf);
		SCPI_ResultString(context, numbuf);
	} else {
		printf("invalid number argument\n");
		return SCPI_RES_ERR;
	}
	return SCPI_RES_OK;
}

int dscpi_output_load1(scpi_t *context)
{
	return dscpi_output_load_inner(context, 0);
}

int dscpi_output_load2(scpi_t *context)
{
	return dscpi_output_load_inner(context, 1);
}

int dscpi_apply1Q(scpi_t *context)
{
	SCPI_ResultString(context, "source_apply1Q");
	return SCPI_RES_OK;
}

int dscpi_apply2Q(scpi_t *context)
{
	SCPI_ResultString(context, "source_apply2Q");
	return SCPI_RES_OK;
}

int dscpi_apply_sin_inner(scpi_t *context, int output)
{
	scpi_number_t freq;
	scpi_number_t ampl;
	scpi_number_t offset;
	if (!SCPI_ParamNumber(context, &freq, false)) {
		/* you can only get here if you gave it a bad pointer */
	} else {
		if (freq.type == SCPI_NUM_DEF) {
			freq.value = 1000;
			freq.type = SCPI_NUM_NUMBER;
			freq.unit = SCPI_UNIT_HERTZ;
		}
		if (freq.unit == SCPI_UNIT_NONE) {
			freq.unit = SCPI_UNIT_HERTZ;
		}
		/* handle minimum, maximum, regular*/
	}
	if (!SCPI_ParamNumber(context, &ampl, false)) {
		/* bad pointers */
	} else {
		if (ampl.type == SCPI_NUM_DEF) {
			ampl.type = SCPI_NUM_NUMBER;
			ampl.unit = SCPI_UNIT_VOLT;
			ampl.value = 0.1; // 100 mV by default
		}
		if (ampl.unit == SCPI_UNIT_NONE) {
			ampl.unit = SCPI_UNIT_VOLT;
		}
		/* handle minimum, maximum, */
		/* TODO somehow work out how to handle Vrms units here ?*/
	}
	if (!SCPI_ParamNumber(context, &offset, false)) {
		/* bad pointers */
	} else {
		if (offset.type == SCPI_NUM_DEF) {
			offset.type = SCPI_NUM_NUMBER;
			offset.unit = SCPI_UNIT_VOLT;
			offset.value = 0;
		}
		if (offset.unit == SCPI_UNIT_NONE) {
			offset.unit = SCPI_UNIT_VOLT;
		}
		/* handle minimum, maximum */
	}
	if (output) {
		SCPI_ResultString(context, "source_apply2_sin");
	} else {
		SCPI_ResultString(context, "source_apply1_sin");
	}
	SCPI_NumberToStr(context, &freq, numbuf, sizeof (numbuf));
	printf("setting output %d to sin wave freq: %s", output + 1, numbuf);
	SCPI_ResultString(context, numbuf);
	SCPI_NumberToStr(context, &ampl, numbuf, sizeof (numbuf));
	printf("ampl: %s", numbuf);
	SCPI_ResultString(context, numbuf);
	SCPI_NumberToStr(context, &offset, numbuf, sizeof (numbuf));
	printf("offset: %s", numbuf);
	SCPI_ResultString(context, numbuf);
	return SCPI_RES_OK;
}

int dscpi_apply_sin1(scpi_t *context)
{
	return dscpi_apply_sin_inner(context, 0);
}

int dscpi_apply_sin2(scpi_t *context)
{
	return dscpi_apply_sin_inner(context, 1);
}

////////// END OF TEST IMPLS



scpi_command_t scpi_commands[] = {
	{ .pattern = "*IDN?", .callback = SCPI_CoreIdnQ,},
	{ .pattern = "OUTPut?", .callback = dscpi_output1Q,},
	{ .pattern = "OUTP1?", .callback = dscpi_output1Q,},
	{ .pattern = "OUTPUT1?", .callback = dscpi_output1Q,},
	{ .pattern = "OUTP2?", .callback = dscpi_output2Q,},
	{ .pattern = "OUTPUT2?", .callback = dscpi_output2Q,},

	{ .pattern = "OUTPut", .callback = dscpi_output1,},
	{ .pattern = "OUTP1", .callback = dscpi_output1,},
	{ .pattern = "OUTPUT1", .callback = dscpi_output1,},
	{ .pattern = "OUTP2", .callback = dscpi_output2,},
	{ .pattern = "OUTPUT2", .callback = dscpi_output2,},

	{ .pattern = "OUTPut:LOAD", .callback = dscpi_output_load1,},
	{ .pattern = "OUTP1:LOAD", .callback = dscpi_output_load1,},
	{ .pattern = "OUTPUT1:LOAD", .callback = dscpi_output_load1,},
	{ .pattern = "OUTP2:LOAD", .callback = dscpi_output_load2,},
	{ .pattern = "OUTPUT2:LOAD", .callback = dscpi_output_load2,},
	/* Ignoring load query for now :( */

	{.pattern = "SOURce:APPLy?", .callback = dscpi_apply1Q,},
	{.pattern = "SOUR1:APPLy?", .callback = dscpi_apply1Q,},
	{.pattern = "SOUR2:APPLy?", .callback = dscpi_apply2Q,},
	{.pattern = "SOURce1:APPLy?", .callback = dscpi_apply1Q,},
	{.pattern = "SOURce2:APPLy?", .callback = dscpi_apply2Q,},

	{.pattern = "SOURce:APPLy:SIN", .callback = dscpi_apply_sin1,},
	{.pattern = "SOUR1:APPLy:SIN", .callback = dscpi_apply_sin1,},
	{.pattern = "SOUR2:APPLy:SIN", .callback = dscpi_apply_sin2,},
	{.pattern = "SOURce1:APPLy:SIN", .callback = dscpi_apply_sin2,},
	{.pattern = "SOURce2:APPLy:SIN", .callback = dscpi_apply_sin2,},


	SCPI_CMD_LIST_END
};


scpi_interface_t scpi_interface = {
	.write = scpi_impl_write,
	.error = scpi_impl_error,
	.reset = NULL,
	.test = NULL,
	.control = NULL,
};

scpi_t scpi_context = {
	.cmdlist = scpi_commands,
	.buffer =
	{
		.length = SCPI_INPUT_BUFFER_LENGTH,
		.data = scpi_input_buffer,
	},
	.interface = &scpi_interface,
	//    .registers = scpi_regs,
	.units = scpi_units_def,
	.special_numbers = scpi_special_numbers_def,
	.idn =
	{"hohoMANU", "kproduct", NULL, "kserial"}
};

size_t scpi_impl_write(scpi_t * context, const char * data, size_t len)
{
	(void) context;
	printf("<scpi_write: %s>\n", data);

	memcpy(obuf + obuf_idx, data, len);
	obuf_idx += len;
	obuf[obuf_idx] = '\0';
	return len;
}

int scpi_impl_error(scpi_t * context, int_fast16_t err)
{
	(void) context;

	fprintf(stderr, "**ERROR: %d, \"%s\"\r\n", (int32_t) err, SCPI_ErrorTranslate(err));
	return 0;
}

static void output_buffer_clear(void)
{
	memset(obuf, 0, sizeof (obuf));
	obuf_idx = 0;
}

int init_suite(void)
{
	SCPI_Init(&scpi_context);
	return 0;
}

int clean_suite(void)
{
	return 0;
}

#define DDDTEST_COMMAND_MATCH(data, output) {                           \
    SCPI_Input(&scpi_context, data, strlen(data));              \
    SCPI_Input(&scpi_context, NULL, 0);              \
    printf(obuf);					\
    CU_ASSERT_STRING_EQUAL(output, obuf);              \
    output_buffer_clear();                                      \
}

void TEST_COMMAND_MATCH(char *data, char* output)
{
	SCPI_Input(&scpi_context, data, strlen(data));
	SCPI_Input(&scpi_context, NULL, 0);
	printf("obuf=<%s>\n", obuf);
	CU_ASSERT_STRING_EQUAL(output, obuf);
	output_buffer_clear();
}

void testScpi_glue_input()
{
	output_buffer_clear();
	TEST_COMMAND_MATCH("*IDN?", "hohoMANU, kproduct, 0, kserial\r\n");
}

void test_output_matches()
{
	output_buffer_clear();
	TEST_COMMAND_MATCH("OUTP?", "output1query\r\n");
	TEST_COMMAND_MATCH("OUTP1?", "output1query\r\n");
	TEST_COMMAND_MATCH("OUTPUT?", "output1query\r\n");
	TEST_COMMAND_MATCH("OUTPUT1?", "output1query\r\n");
	TEST_COMMAND_MATCH("OUTP2?", "output2query\r\n");
	TEST_COMMAND_MATCH("OUTPUT2?", "output2query\r\n");

	TEST_COMMAND_MATCH("outp off", "output1, off\r\n");
	TEST_COMMAND_MATCH("outp on", "output1, on\r\n");
	TEST_COMMAND_MATCH("outp1 off", "output1, off\r\n");
	TEST_COMMAND_MATCH("outp1 on", "output1, on\r\n");
	TEST_COMMAND_MATCH("output1 off", "output1, off\r\n");
	TEST_COMMAND_MATCH("output1 on", "output1, on\r\n");
	TEST_COMMAND_MATCH("outp2 off", "output2, off\r\n");
	TEST_COMMAND_MATCH("outp2 on", "output2, on\r\n"); // test bjarni1
	TEST_COMMAND_MATCH("output2 off", "output2, off\r\n");
	TEST_COMMAND_MATCH("output2 on", "output2, on\r\n");

	TEST_COMMAND_MATCH("output2 1", "output2, on\r\n");
	TEST_COMMAND_MATCH("output 0", "output1, off\r\n");
}

void test_output_load()
{
	output_buffer_clear();
	TEST_COMMAND_MATCH("outp1:load inf", "output1_load, infinite\r\n"); // test bjarni2
	TEST_COMMAND_MATCH("output2:load default", "output2_load, default\r\n");
	TEST_COMMAND_MATCH("output2:load 2.5 kohm", "output2_load, 2500 OHM\r\n");
	TEST_COMMAND_MATCH("output2:load 25 ohm", "output2_load, 25 OHM\r\n");
	TEST_COMMAND_MATCH("output2:load 25", "output2_load, 25 OHM\r\n");
}

void test_bjarni3()
{
	output_buffer_clear();
	//TEST_COMMAND_MATCH("sour1:appl:sin 50Hz,0.456Vrms,0", "1,sin,50 Hz, 0.456 VRMS, 0") // bjarni3
	TEST_COMMAND_MATCH("sour1:appl:sin 50Hz,456mV,0", "source_apply1_sin, 50 HZ, 0.456 V, 0 V\r\n");
	TEST_COMMAND_MATCH("sour1:appl:sin", "source_apply1_sin, 1000 HZ, 0.1 V, 0 V\r\n");
	TEST_COMMAND_MATCH("sour1:appl:sin 123", "source_apply1_sin, 123 HZ, 0.1 V, 0 V\r\n");
	TEST_COMMAND_MATCH("sour2:appl:sin 1.2 KHz", "source_apply2_sin, 1200 HZ, 0.1 V, 0 V\r\n");
	TEST_COMMAND_MATCH("sour1:appl:sin 1.2 KHz, 0.200", "source_apply1_sin, 1200 HZ, 0.2 V, 0 V\r\n");
	TEST_COMMAND_MATCH("source2:apply:sin 1.2 KHz, 0.200, 0.5", "source_apply2_sin, 1200 HZ, 0.2 V, 0.5 V\r\n");
}

void test_applyq()
{
	output_buffer_clear();
	TEST_COMMAND_MATCH("sour1:appl?", "source_apply1Q\r\n");
	TEST_COMMAND_MATCH("sour:appl?", "source_apply1Q\r\n");
	TEST_COMMAND_MATCH("source:apply?", "source_apply1Q\r\n");
	TEST_COMMAND_MATCH("source2:apply?", "source_apply2Q\r\n");
	TEST_COMMAND_MATCH("SOUR2:APPL?", "source_apply2Q\r\n");
}

int main()
{
	CU_pSuite pSuite = NULL;

	/* Initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* Add a suite to the registry */
	pSuite = CU_add_suite("newcunittest", init_suite, clean_suite);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Add the tests to the suite */
	if (0 ||
		(NULL == CU_add_test(pSuite, "testScpi_glue_input", testScpi_glue_input)) ||
		(NULL == CU_add_test(pSuite, "test_output_matches", test_output_matches)) ||
		(NULL == CU_add_test(pSuite, "test_output_load", test_output_load)) ||
		(NULL == CU_add_test(pSuite, "test_bjarni3", test_bjarni3)) ||
		(NULL == CU_add_test(pSuite, "test_applyq", test_applyq)) ||
		0) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}


