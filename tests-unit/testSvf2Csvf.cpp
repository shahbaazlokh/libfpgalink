/* 
 * Copyright (C) 2010 Chris McClelland
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cstdio>
#include <cstring>
#include <UnitTest++.h>
#include <makestuff.h>
#include <libbuffer.h>
#include "../svf2csvf.h"

using namespace std;

void testReadBytes(struct Buffer *buf, const char *expected) {
	FLStatus fStatus;
	char result[1024];
	const uint32 expectedLength = strlen(expected)/2;
	bufZeroLength(buf);
	fStatus = readBytes(buf, expected, NULL);
	CHECK_EQUAL(FL_SUCCESS, fStatus);
	CHECK_EQUAL(expectedLength, buf->length);
	for ( uint32 i = 0; i < 2*buf->length; i++ ) {
		sprintf(result + 2*i, "%02X", buf->data[i]);
	}
	result[2*buf->length] = '\0';
	CHECK_EQUAL(expected, result);
}

TEST(FPGALink_testReadBytes) {
	struct Buffer lineBuf;
	BufferStatus bStatus;
	bStatus = bufInitialise(&lineBuf, 1024, 0x00, NULL);
	CHECK_EQUAL(BUF_SUCCESS, bStatus);

	testReadBytes(&lineBuf, "FE");
	testReadBytes(&lineBuf, "CAFE");
	testReadBytes(&lineBuf, "F00D1E");
	testReadBytes(&lineBuf, "DEADF00D");
	testReadBytes(&lineBuf, "CAFEF00D1E");
	testReadBytes(&lineBuf, "DEADCAFEBABE");

	bufDestroy(&lineBuf);
}

void testShift(const char *line, uint32 lineBits, const char *head, uint32 headBits, const char *tail, uint32 tailBits, const char *expectedHex, uint32 expectedLength) {
	struct Buffer lineBuf;
	struct Buffer headBuf;
	struct Buffer tailBuf;
	BufferStatus bStatus;
	FLStatus fStatus;
	char resultHex[1024];
	bStatus = bufInitialise(&lineBuf, 1024, 0x00, NULL);
	CHECK_EQUAL(BUF_SUCCESS, bStatus);
	bStatus = bufInitialise(&headBuf, 1024, 0x00, NULL);
	CHECK_EQUAL(BUF_SUCCESS, bStatus);
	bStatus = bufInitialise(&tailBuf, 1024, 0x00, NULL);
	CHECK_EQUAL(BUF_SUCCESS, bStatus);
	fStatus = readBytes(&lineBuf, line, NULL);
	CHECK_EQUAL(FL_SUCCESS, fStatus);
	fStatus = readBytes(&headBuf, head, NULL);
	CHECK_EQUAL(FL_SUCCESS, fStatus);
	fStatus = readBytes(&tailBuf, tail, NULL);
	CHECK_EQUAL(FL_SUCCESS, fStatus);
	fStatus = headTail(&lineBuf, &headBuf, &tailBuf, lineBits, headBits, tailBits, NULL);
	CHECK_EQUAL(FL_SUCCESS, fStatus);
	CHECK_EQUAL(strlen(expectedHex)/2, lineBuf.length);
	for ( uint32 i = 0; i < 2*lineBuf.length; i++ ) {
        sprintf(resultHex + 2*i, "%02X", lineBuf.data[i]);
    }
    resultHex[2*lineBuf.length] = '\0';
    CHECK_EQUAL(expectedHex, resultHex);
	bufDestroy(&tailBuf);
	bufDestroy(&headBuf);
	bufDestroy(&lineBuf);
}	

TEST(FPGALink_testShift) {
	testShift(
		"F1C2E093", 32,
		"00", 0,
		"06", 3,
		"06F1C2E093", 35
	);
	testShift(
		"F1C2E093", 32,
		"01", 1,
		"06", 3,
		"0DE385C127", 36
	);
	testShift(
		"F1C2E093", 32,
		"02", 2,
		"06", 3,
		"1BC70B824E", 37
	);
	testShift(
		"F1C2E093", 32,
		"06", 3,
		"06", 3,
		"378E17049E", 38
	);
	testShift(
		"F1C2E093", 32,
		"0A", 4,
		"06", 3,
		"6F1C2E093A", 39
	);
	testShift(
		"F1C2E093", 32,
		"15", 5,
		"06", 3,
		"DE385C1275", 40
	);
	testShift(
		"F1C2E093", 32,
		"25", 6,
		"06", 3,
		"01BC70B824E5", 41
	);
	testShift(
		"F1C2E093", 32,
		"75", 7,
		"06", 3,
		"0378E17049F5", 42
	);
	testShift(
		"F1C2E093", 32,
		"e5", 8,
		"06", 3,
		"06F1C2E093E5", 43
	);
	testShift(
		"F1C2E093", 32,
		"0115", 9,
		"06", 3,
		"0DE385C12715", 44
	);
	testShift(
		"F1C2E093", 32,
		"0315", 10,
		"06", 3,
		"1BC70B824F15", 45
	);
	testShift(
		"F1C2E093", 32,
		"0715", 11,
		"06", 3,
		"378E17049F15", 46
	);
	testShift(
		"F1C2E093", 32,
		"0c15", 12,
		"06", 3,
		"6F1C2E093C15", 47
	);
	testShift(
		"F1C2E093", 32,
		"1c15", 13,
		"06", 3,
		"DE385C127C15", 48
	);
	testShift(
		"F1C2E093", 32,
		"2c15", 14,
		"06", 3,
		"01BC70B824EC15", 49
	);
	testShift(
		"F1C2E093", 32,
		"4015", 15,
		"06", 3,
		"0378E17049C015", 50
	);
	testShift(
		"F1C2E093", 32,
		"8015", 16,
		"06", 3,
		"06F1C2E0938015", 51
	);
	testShift(
		"F1C2E093", 32,
		"018015", 17,
		"06", 3,
		"0DE385C1278015", 52
	);
	testShift(
		"F1C2E093", 32,
		"00", 0,
		"0A", 4,
		"0AF1C2E093", 36
	);
	testShift(
		"F1C2E093", 32,
		"01", 1,
		"0A", 4,
		"15E385C127", 37
	);
	testShift(
		"F1C2E093", 32,
		"02", 2,
		"0A", 4,
		"2BC70B824E", 38
	);
	testShift(
		"F1C2E093", 32,
		"06", 3,
		"0A", 4,
		"578E17049E", 39
	);
	testShift(
		"F1C2E093", 32,
		"0A", 4,
		"0A", 4,
		"AF1C2E093A", 40
	);
	testShift(
		"F1C2E093", 32,
		"15", 5,
		"0A", 4,
		"015E385C1275", 41
	);
	testShift(
		"F1C2E093", 32,
		"25", 6,
		"0A", 4,
		"02BC70B824E5", 42
	);
	testShift(
		"F1C2E093", 32,
		"75", 7,
		"0A", 4,
		"0578E17049F5", 43
	);
	testShift(
		"F1C2E093", 32,
		"e5", 8,
		"0A", 4,
		"0AF1C2E093E5", 44
	);
	testShift(
		"F1C2E093", 32,
		"0115", 9,
		"0A", 4,
		"15E385C12715", 45
	);
	testShift(
		"F1C2E093", 32,
		"0315", 10,
		"0A", 4,
		"2BC70B824F15", 46
	);
	testShift(
		"F1C2E093", 32,
		"0715", 11,
		"0A", 4,
		"578E17049F15", 47
	);
	testShift(
		"F1C2E093", 32,
		"0c15", 12,
		"0A", 4,
		"AF1C2E093C15", 48
	);
	testShift(
		"F1C2E093", 32,
		"1c15", 13,
		"0A", 4,
		"015E385C127C15", 49
	);
	testShift(
		"F1C2E093", 32,
		"2c15", 14,
		"0A", 4,
		"02BC70B824EC15", 50
	);
	testShift(
		"F1C2E093", 32,
		"4015", 15,
		"0A", 4,
		"0578E17049C015", 51
	);
	testShift(
		"F1C2E093", 32,
		"8015", 16,
		"0A", 4,
		"0AF1C2E0938015", 52
	);
	testShift(
		"F1C2E093", 32,
		"018015", 17,
		"0A", 4,
		"15E385C1278015", 53
	);
	testShift(
		"F1C2E093", 32,
		"00", 0,
		"0135", 9,
		"0135F1C2E093", 41
	);
	testShift(
		"F1C2E093", 32,
		"01", 1,
		"0135", 9,
		"026BE385C127", 42
	);
	testShift(
		"F1C2E093", 32,
		"02", 2,
		"0135", 9,
		"04D7C70B824E", 43
	);
	testShift(
		"F1C2E093", 32,
		"06", 3,
		"0135", 9,
		"09AF8E17049E", 44
	);
	testShift(
		"F1C2E093", 32,
		"0A", 4,
		"0135", 9,
		"135F1C2E093A", 45
	);
	testShift(
		"F1C2E093", 32,
		"15", 5,
		"0135", 9,
		"26BE385C1275", 46
	);
	testShift(
		"F1C2E093", 32,
		"25", 6,
		"0135", 9,
		"4D7C70B824E5", 47
	);
	testShift(
		"F1C2E093", 32,
		"75", 7,
		"0135", 9,
		"9AF8E17049F5", 48
	);
	testShift(
		"F1C2E093", 32,
		"e5", 8,
		"0135", 9,
		"0135F1C2E093E5", 49
	);
	testShift(
		"F1C2E093", 32,
		"0115", 9,
		"0135", 9,
		"026BE385C12715", 50
	);
	testShift(
		"F1C2E093", 32,
		"0315", 10,
		"0135", 9,
		"04D7C70B824F15", 51
	);
	testShift(
		"F1C2E093", 32,
		"0715", 11,
		"0135", 9,
		"09AF8E17049F15", 52
	);
	testShift(
		"F1C2E093", 32,
		"0c15", 12,
		"0135", 9,
		"135F1C2E093C15", 53
	);
	testShift(
		"F1C2E093", 32,
		"1c15", 13,
		"0135", 9,
		"26BE385C127C15", 54
	);
	testShift(
		"F1C2E093", 32,
		"2c15", 14,
		"0135", 9,
		"4D7C70B824EC15", 55
	);
	testShift(
		"F1C2E093", 32,
		"4015", 15,
		"0135", 9,
		"9AF8E17049C015", 56
	);
	testShift(
		"F1C2E093", 32,
		"8015", 16,
		"0135", 9,
		"0135F1C2E0938015", 57
	);
	testShift(
		"F1C2E093", 32,
		"018015", 17,
		"0135", 9,
		"026BE385C1278015", 58
	);
	testShift(
		"71C2E093", 31,
		"00", 0,
		"0135", 9,
		"9AF1C2E093", 40
	);
	testShift(
		"71C2E093", 31,
		"01", 1,
		"0135", 9,
		"0135E385C127", 41
	);
	testShift(
		"71C2E093", 31,
		"02", 2,
		"0135", 9,
		"026BC70B824E", 42
	);
	testShift(
		"71C2E093", 31,
		"06", 3,
		"0135", 9,
		"04D78E17049E", 43
	);
	testShift(
		"71C2E093", 31,
		"0A", 4,
		"0135", 9,
		"09AF1C2E093A", 44
	);
	testShift(
		"71C2E093", 31,
		"15", 5,
		"0135", 9,
		"135E385C1275", 45
	);
	testShift(
		"71C2E093", 31,
		"25", 6,
		"0135", 9,
		"26BC70B824E5", 46
	);
	testShift(
		"71C2E093", 31,
		"75", 7,
		"0135", 9,
		"4D78E17049F5", 47
	);
	testShift(
		"71C2E093", 31,
		"e5", 8,
		"0135", 9,
		"9AF1C2E093E5", 48
	);
	testShift(
		"71C2E093", 31,
		"0115", 9,
		"0135", 9,
		"0135E385C12715", 49
	);
	testShift(
		"71C2E093", 31,
		"0315", 10,
		"0135", 9,
		"026BC70B824F15", 50
	);
	testShift(
		"71C2E093", 31,
		"0715", 11,
		"0135", 9,
		"04D78E17049F15", 51
	);
	testShift(
		"71C2E093", 31,
		"0c15", 12,
		"0135", 9,
		"09AF1C2E093C15", 52
	);
	testShift(
		"71C2E093", 31,
		"1c15", 13,
		"0135", 9,
		"135E385C127C15", 53
	);
	testShift(
		"71C2E093", 31,
		"2c15", 14,
		"0135", 9,
		"26BC70B824EC15", 54
	);
	testShift(
		"71C2E093", 31,
		"4015", 15,
		"0135", 9,
		"4D78E17049C015", 55
	);
	testShift(
		"71C2E093", 31,
		"8015", 16,
		"0135", 9,
		"9AF1C2E0938015", 56
	);
	testShift(
		"71C2E093", 31,
		"018015", 17,
		"0135", 9,
		"0135E385C1278015", 57
	);
	//#include "blah.h"
	testShift(
		"5423DF3A8F129B2C91B9425D", 95,
		"", 0,
		"01", 1,
		"D423DF3A8F129B2C91B9425D", 96
	);
	testShift(
		"5423DF3A8F129B2C91B9425D", 95,
		"01", 1,
		"", 0,
		"A847BE751E253659237284BB", 96
	);
}

void parseString(ParseContext *cxt, const char *str, struct Buffer *csvfBuf) {
	Buffer line;
	BufferStatus bStatus;
	FLStatus fStatus;
	const uint32 lineLen = strlen(str) + 1;
	bStatus = bufInitialise(&line, lineLen, 0, NULL);
	CHECK_EQUAL(BUF_SUCCESS, bStatus);
	bStatus = bufAppendBlock(&line, (const uint8 *)str, lineLen, NULL);
	CHECK_EQUAL(BUF_SUCCESS, bStatus);
	fStatus = parseLine(cxt, &line, csvfBuf, NULL);
	CHECK_EQUAL(FL_SUCCESS, fStatus);
	bufDestroy(&line);
}

void renderBuffer(char *&p, const Buffer *buf) {
	const char *src = (const char *)buf->data;
	uint32 length = buf->length;
	*p++ = '{';
	while ( length-- ) {
		sprintf(p, "%02X", *src & 0xFF);
		src++;
		p += 2;
	}
	*p++ = '}';
}

void renderBitStore(char *&p, const BitStore *store) {
	sprintf(p, "%d, ", store->numBits);
	p += strlen(p);
	renderBuffer(p, &store->tdi);
	*p++ = ','; *p++ = ' ';
	renderBuffer(p, &store->tdo);
	*p++ = ','; *p++ = ' ';
	renderBuffer(p, &store->mask);
}

const char *renderCxt(const ParseContext *cxt) {
	static char result[1024];
	char *p = result;

	renderBitStore(p, &cxt->dataHead);
	*p++ = ','; *p++ = ' ';
	renderBitStore(p, &cxt->insnHead);
	*p++ = ','; *p++ = ' ';

	renderBitStore(p, &cxt->dataBody);
	*p++ = ','; *p++ = ' ';
	renderBitStore(p, &cxt->insnBody);
	*p++ = ','; *p++ = ' ';

	renderBitStore(p, &cxt->dataTail);
	*p++ = ','; *p++ = ' ';
	renderBitStore(p, &cxt->insnTail);
	
	*p = '\0';

	return result;
}

TEST(FPGALink_testParse) {
	FLStatus fStatus;
	struct ParseContext cxt;
	struct Buffer csvfBuf;
	BufferStatus bStatus;
	bStatus = bufInitialise(&csvfBuf, 1024, 0x00, NULL);
	CHECK_EQUAL(BUF_SUCCESS, bStatus);
	fStatus = cxtInitialise(&cxt, NULL);
	CHECK_EQUAL(FL_SUCCESS, fStatus);

	// HDR, HIR, SDR, SIR, TDR, TIR
	parseString(&cxt, "HDR 8 TDI (aa)", &csvfBuf);
	CHECK_EQUAL(
		"8, {AA}, {00}, {00}, "    // HDR
		"0, {}, {}, {}, "          // HIR
		"0, {}, {}, {}, "          // SDR
		"0, {}, {}, {}, "          // SIR
		"0, {}, {}, {}, "          // TDR
		"0, {}, {}, {}",           // TIR
		renderCxt(&cxt));

	// HDR, HIR, SDR, SIR, TDR, TIR
	parseString(&cxt, "HDR 8 MASK (55)", &csvfBuf);
	CHECK_EQUAL(
		"8, {AA}, {00}, {55}, "    // HDR
		"0, {}, {}, {}, "          // HIR
		"0, {}, {}, {}, "          // SDR
		"0, {}, {}, {}, "          // SIR
		"0, {}, {}, {}, "          // TDR
		"0, {}, {}, {}",           // TIR
		renderCxt(&cxt));

	// HDR, HIR, SDR, SIR, TDR, TIR
	parseString(&cxt, "HDR 6 TDI (3A)", &csvfBuf);
	CHECK_EQUAL(
		"6, {3A}, {00}, {00}, "    // HDR
		"0, {}, {}, {}, "          // HIR
		"0, {}, {}, {}, "          // SDR
		"0, {}, {}, {}, "          // SIR
		"0, {}, {}, {}, "          // TDR
		"0, {}, {}, {}",           // TIR
		renderCxt(&cxt));

	cxtDestroy(&cxt);
	bufDestroy(&csvfBuf);
}
