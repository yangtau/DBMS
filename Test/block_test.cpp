﻿#include "CppUnitTest.h"
#include "stdafx.h"
#include "../Project/block.h"
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest {
	TEST_CLASS(BlockUnitTest) {
public:


	TEST_METHOD(BlockCheck) {
		short *buffer = (short*)malloc(BLOCK_SIZE);
		for (int i = 0; i < BLOCK_SIZE / 2; i++) {
			srand(i);
			buffer[i] = (short)rand();
		}
		RecordBlock *block = (RecordBlock*)buffer;
		uint16_t checksum = block->header.compute();
		block->header.checksum = checksum;
		Assert::AreEqual(1, block->header.check());
		free(buffer);
	}


	TEST_METHOD(BlockAddRecord) {
		RecordBlock *block = (RecordBlock*)malloc(BLOCK_SIZE);
		char data[10] = { 6, 0, 0, 0, 'H', 'e', 'l', 'l', 'o', '\0' };
		unsigned size = 6;
		block->header.count = 0;
		block->header.free = sizeof(BlockHeader);
		// add multi records
		int len = 300;
		for (int i = 0; i < len; i++) {
			Assert::AreEqual(1, block->addRecord((Record*)data));
		}
		// count
		Assert::AreEqual(len, (int)block->header.count);
		// offset
		unsigned exceptOffset = sizeof(BlockHeader) + (size + sizeof(RecordHeader)) * (len - 1);
		unsigned offset = block->getTailer()->slots[0];
		Assert::AreEqual(exceptOffset, offset);
		// free
		unsigned freeP = sizeof(BlockHeader) + (size + sizeof(RecordHeader)) * len;
		unsigned actu = block->header.free;
		Assert::AreEqual(freeP, actu);
		free(block);
	}

	TEST_METHOD(BLOCKGetRecord) {
		RecordBlock *block = (RecordBlock*)malloc(BLOCK_SIZE);
		char data[10] = { 6, 0, 0, 0, 'H', 'e', 'l', 'l', 'o', '\0' };
		unsigned size = 6;
		block->header.count = 0;
		block->header.free = sizeof(BlockHeader);
		// add record
		block->header.count = 0;
		block->header.free = sizeof(BlockHeader);
		int len = 300;
		for (int i = 0; i < len; i++) {
			Assert::AreEqual(1, block->addRecord((Record*)data));
		}
		for (int i = 0; i < len; i++) {
			// check
			Record * rec = (Record*)block->getRecord(i);
			Assert::AreEqual(size, (unsigned)(rec->header.size));
			Assert::AreEqual(std::string(data + 4), std::string((char*)rec->getData()));
		}
		free(block);
	}

	TEST_METHOD(BLOCKDelRecord) {
		RecordBlock *block = (RecordBlock*)malloc(BLOCK_SIZE);
		char data[15] = { 11, 0, 0, 0, 'H', 'e', 'l', 'l', 'o','w','o','r' ,'l','d','\0' };
		unsigned size = 11;
		block->header.count = 0;
		block->header.free = sizeof(BlockHeader);
		// add record
		block->header.count = 0;
		block->header.free = sizeof(BlockHeader);
		int len = 200;
		for (int i = 0; i < len; i++) {
			Assert::AreEqual(1, block->addRecord((Record*)data));
		}
		for (int i = 0; i < len; i++) {
			Assert::AreEqual(1, block->delRecord(i));
		}
		Assert::AreEqual(len, (int)block->header.count);
		for (int i = 0; i < len; i++) {
			Assert::AreEqual(0, (int)block->getTailer()->slots[i]);
		}
		free(block);
	}

	TEST_METHOD(BLOCKUpdateRecord) {
		RecordBlock *block = (RecordBlock*)malloc(BLOCK_SIZE);
		char data[10] = { 6, 0, 0, 0, 'H', 'e', 'l', 'l', 'o', '\0' };
		unsigned size = 6;
		block->header.count = 0;
		block->header.free = sizeof(BlockHeader);
		int len = 200;
		for (int i = 0; i < len; i++) {
			Assert::AreEqual(1, block->addRecord((Record*)data));
		}
		char newData[] = { 8, 0, 0, 0, 'h', 'i', 'h', 'i','h','o','e', '\0' };
		for (int i = 0; i < len; i++) {
			Assert::AreEqual(1, block->updateRecord(i, (Record*)newData));
		}
		for (int i = 0; i < len; i++) {
			Record * rec = (Record*)block->getRecord(i);
			Assert::AreEqual(std::string(newData + 4), std::string((char*)rec->getData()));
		}
		free(block);
	}

	};
}
