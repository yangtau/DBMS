#pragma once
#include "btree.h"
#include <iostream>
#include <string>

#define assert_equal(a, b, msg)                                                \
  do {                                                                         \
    if (!(a == b)) {                                                           \
      fprintf(stderr, "#func: %s#line:%d ", __func__, __LINE__);               \
      fprintf(stderr, "%s\n", msg);                                            \
      getchar();                                                               \
      exit(-1);                                                                \
    }                                                                          \
  } while (0)

using namespace std;

string s = "HelloWorld";

int cmp(const void *a, const void *b, void *) { return *(int *)a - *(int *)b; }

void insertIntoBTree(BTree *btree, int a, int b) {
    for (int i = a; i < b; i++) {
        string value = to_string(i) + s + to_string(i * 2);
        int res = btree->put(&i, value.c_str(), value.length() + 1);
        assert_equal(1, res, to_string(i).c_str());
    }
}

void testPutAndQuery() {
    {
        // put
        BTree b(sizeof(int), cmp);
        b.create("test.db");
        assert_equal(1, b.open("test.db"), "");
        insertIntoBTree(&b, 400, 500);
        insertIntoBTree(&b, 0, 200);
        insertIntoBTree(&b, 250, 300);
    }

    {
        // put duplicate key
        BTree b(sizeof(int), cmp);
        assert_equal(1, b.open("test.db"), "");
        int key = 401;
        assert_equal(0, b.put(&key, NULL, 0), "");
        key = 20;
        assert_equal(0, b.put(&key, NULL, 0), "");
        key = 260;
        assert_equal(0, b.put(&key, NULL, 0), "");
    }
    {
        // [400, 500)
        // [0, 200)
        // [250, 300)
        BTree bt(sizeof(int), cmp);
        assert_equal(1, bt.open("test.db"), "");
        BTree::Iterator it = bt.iterator();

        for (int i = 0; i < 200; i++) {
            assert_equal(1, it.locate(&i, BTree::EXACT_KEY), "");
            string eval = to_string(i) + s + to_string(i * 2);
            string rval = string((char *)it.getValue());
            assert_equal(eval, rval, (eval + "!=" + rval).c_str());
        }

        //[400, 500)
        int a = 400;
        assert_equal(1, it.locate(&a, BTree::EXACT_KEY), "");
        do {
            string eval = to_string(a) + s + to_string(a * 2);
            a++;
            string rval = string((char *)it.getValue());
            assert_equal(eval, rval, (eval + "!=" + rval).c_str());
        } while (it.next());
        assert_equal(a, 500, "");

        // [250, 300)
        int b = 300;
        assert_equal(1, it.locate(&b, BTree::KEY_OR_PREV), "");
        b--;
        do {
            string eval = to_string(b) + s + to_string(b * 2);
            b--;
            if (b < 250) {
                break;
            }
            string rval = string((char *)it.getValue());
            assert_equal(eval, rval, (eval + "!=" + rval).c_str());
        } while (it.prev());
        assert_equal(249, b, "");
        assert_equal(1, it.prev(), "");
        assert_equal(199, *(int *)it.getKey(), "");
    }
}

// testPutAndQuery must run before this test.
void testIterator() {
    // [400, 500)
    // [0, 200)
    // [250, 300)
    BTree btree(sizeof(int), cmp);
    assert_equal(1, btree.open("test.db"), "");
    BTree::Iterator it = btree.iterator();

    {
        // first & last
        assert_equal(1, it.first(), "");
        assert_equal(0, *(int *)it.getKey(), "");

        assert_equal(1, it.last(), "");
        assert_equal(499, *(int *)it.getKey(), "");
    }

    {
        // key or next
        int key = -1;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_NEXT), "");
        assert_equal(key + 1, *(int *)it.getKey(), "");
        assert_equal(0, it.prev(), "");
        assert_equal(1, it.next(), "");

        key = 200;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_NEXT), "");
        assert_equal(250, *(int *)it.getKey(), "");
        assert_equal(1, it.prev(), "");
        assert_equal(1, it.next(), "");

        key = 500;
        assert_equal(0, it.locate(&key, BTree::KEY_OR_NEXT), "");
    }
    {
        // key or prev
        // [400, 500)
        // [0, 200)
        // [250, 300)
        int key = 400;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_PREV), "");
        assert_equal(400, *(int *)it.getKey(), "");
        assert_equal(1, it.prev(), "");
        assert_equal(299, *(int *)it.getKey(), "");

        key = 500;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_PREV), "");
        assert_equal(499, *(int *)it.getKey(), "");
        // next
        assert_equal(0, it.next(), "");

        key = 250;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_PREV), "");
        assert_equal(250, *(int *)it.getKey(), "");
        // prev
        assert_equal(1, it.prev(), "");
        assert_equal(199, *(int *)it.getKey(), "");

        key = 600;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_PREV), "");
        assert_equal(499, *(int *)it.getKey(), "");

        key = 0;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_PREV), "");
        assert_equal(0, *(int *)it.getKey(), "");

        key = -1;
        assert_equal(0, it.locate(&key, BTree::KEY_OR_PREV), "");
    }
    {
        // afer key
        // [400, 500)
        // [0, 200)
        // [250, 300)
        int key = 0;
        assert_equal(1, it.locate(&key, BTree::AFTER_KEY), "");
        assert_equal(1, *(int *)it.getKey(), "");

        key = 199;
        assert_equal(1, it.locate(&key, BTree::AFTER_KEY), "");
        assert_equal(250, *(int *)it.getKey(), "");

        key = 499;
        assert_equal(0, it.locate(&key, BTree::AFTER_KEY), "");

        key = -200;
        assert_equal(1, it.locate(&key, BTree::AFTER_KEY), "");
        assert_equal(0, *(int *)it.getKey(), "");
    }
    {
        // before key
        // [400, 500)
        // [0, 200)
        // [250, 300)
        int key = -200;
        assert_equal(0, it.locate(&key, BTree::BEFROE_KEY), "");

        key = 500;
        assert_equal(1, it.locate(&key, BTree::BEFROE_KEY), "");
        assert_equal(499, *(int *)it.getKey(), "");

        key = 400;
        assert_equal(1, it.locate(&key, BTree::BEFROE_KEY), "");
        assert_equal(299, *(int *)it.getKey(), "");

        key = 225;
        assert_equal(1, it.locate(&key, BTree::BEFROE_KEY), "");
        assert_equal(199, *(int *)it.getKey(), "");
    }
}

void testRemove() {
    { // [400, 500)
      // [0, 200)
      // [250, 300)
        BTree bt(sizeof(int), cmp);
        assert_equal(1, bt.open("test.db"), "");
        BTree::Iterator it = bt.iterator();
        assert_equal(1, it.first(), "");

        do {
            int key = *(int *)it.getKey();
            if (key % 2 == 1)
                it.remove();
        } while (it.next());
    }
    {
        // [0, 200)
        // [250, 300)
        // [400, 500)
        BTree bt(sizeof(int), cmp);
        assert_equal(1, bt.open("test.db"), "");
        BTree::Iterator it = bt.iterator();

        for (int i = 0; i < 200; i++) {
            if (i % 2 == 1) {
                assert_equal(0, it.locate(&i, BTree::EXACT_KEY), "");
                continue;
            }
            assert_equal(1, it.locate(&i, BTree::EXACT_KEY), "");
            string eval = to_string(i) + s + to_string(i * 2);
            string rval = string((char *)it.getValue());
            assert_equal(eval, rval, (eval + "!=" + rval).c_str());
        }

        //[400, 500)
        int a = 400;
        assert_equal(1, it.locate(&a, BTree::EXACT_KEY), "");
        do {
            string eval = to_string(a) + s + to_string(a * 2);
            a += 2;
            string rval = string((char *)it.getValue());
            assert_equal(eval, rval, (eval + "!=" + rval).c_str());
        } while (it.next());
        assert_equal(500, a, "");

        // [250, 300)
        int b = 300;
        assert_equal(1, it.locate(&b, BTree::KEY_OR_PREV), "");
        b -= 2;
        do {
            string eval = to_string(b) + s + to_string(b * 2);
            b -= 2;
            if (b < 250) {
                break;
            }
            string rval = string((char *)it.getValue());
            assert_equal(eval, rval, (eval + "!=" + rval).c_str());
        } while (it.prev());
        assert_equal(248, b, "");
        assert_equal(1, it.prev(), "");
        assert_equal(198, *(int *)it.getKey(), "");
    }

    {
        // [0, 200)
        // [250, 300)
        // [400, 500)
        // even
        BTree bt(sizeof(int), cmp);
        assert_equal(1, bt.open("test.db"), "");
        BTree::Iterator it = bt.iterator();
        assert_equal(1, it.first(), "");
        assert_equal(0, *(int *)it.getKey(), "");
        assert_equal(1, it.last(), "");
        assert_equal(498, *(int *)it.getKey(), "");

        int key;
        // key or next
        key = 2;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_NEXT), "");
        assert_equal(2, *(int *)it.getKey(), "");
        key = 251;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_NEXT), "");
        assert_equal(252, *(int *)it.getKey(), "");

        // key or prev
        key = 400;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_PREV), "");
        assert_equal(400, *(int *)it.getKey(), "");
        key = 499;
        assert_equal(1, it.locate(&key, BTree::KEY_OR_PREV), "");
        assert_equal(498, *(int *)it.getKey(), "");

        // after key
        key = 1;
        assert_equal(1, it.locate(&key, BTree::AFTER_KEY), "");
        assert_equal(2, *(int *)it.getKey(), "");
        key = 250;
        assert_equal(1, it.locate(&key, BTree::AFTER_KEY), "");
        assert_equal(252, *(int *)it.getKey(), "");

        // before key
        key = 0;
        assert_equal(0, it.locate(&key, BTree::BEFROE_KEY), "");

        key = 250;
        assert_equal(1, it.locate(&key, BTree::BEFROE_KEY), "");
        assert_equal(198, *(int *)it.getKey(), "");
    }
}

void testReinsert() {
    { // [0, 200)
      // [250, 300)
      // [400, 500)
      // even
        BTree btree(sizeof(int), cmp);
        assert_equal(1, btree.open("test.db"), "");
        for (int i = 0; i < 200; i++) {
            if (i % 2 == 0)
                continue;
            string value = to_string(i) + s + to_string(i * 2);
            int res = btree.put(&i, value.c_str(), value.length() + 1);
            assert_equal(1, res, to_string(i).c_str());
        }
    }
    {
        BTree bt(sizeof(int), cmp);
        assert_equal(1, bt.open("test.db"), "");
        BTree::Iterator it = bt.iterator();

        for (int i = 0; i < 200; i++) {
            assert_equal(1, it.locate(&i, BTree::EXACT_KEY), "");
            string eval = to_string(i) + s + to_string(i * 2);
            string rval = string((char *)it.getValue());
            assert_equal(eval, rval, (eval + "!=" + rval).c_str());
        }
    }
}

void testConstValue() {
    {
        BTree btree(sizeof(int), cmp, NULL, sizeof(int));
        assert_equal(1, btree.create("test.db"), "");
        assert_equal(1, btree.open("test.db"), "");

        for (int i = 0; i < 400; i += 2) {
            int value = i * i + i;
            assert_equal(1, btree.put(&i, &value), "");
        }
    }
    {
        BTree btree(sizeof(int), cmp, NULL, sizeof(int));
        assert_equal(1, btree.open("test.db"), "");
        BTree::Iterator it = btree.iterator();
        int key = 0;
        assert_equal(1, it.locate(&key, BTree::EXACT_KEY), "");
        do {
            assert_equal(key * key + key, *(int *)it.getValue(), "");
            key += 2;
        } while (it.next());
        assert_equal(400, key, "");
    }
}

void testBigData() {
    {
        BTree btree(sizeof(int), cmp, NULL);
        assert_equal(1, btree.create("test.db"), "");
        assert_equal(1, btree.open("test.db"), "");

        for (int i = 0; i < 10000; i++) {
            string value = to_string(i) + "# Write the code. Change the world. #" + to_string(i*i);
            assert_equal(1, btree.put(&i, value.c_str(), value.length() + 1), "");
        }
    }
    {
        BTree btree(sizeof(int), cmp, NULL);
        assert_equal(1, btree.open("test.db"), "");
        BTree::Iterator it = btree.iterator();
        int key = 0;
        assert_equal(1, it.locate(&key, BTree::EXACT_KEY), "");
        do {
            int i = key;
            string value = to_string(i) + "# Write the code. Change the world. #" + to_string(i*i);
            string relval = string((char*)it.getValue());
            assert_equal(value, relval, (value + "!=" + relval).c_str());
            key++;
        } while (it.next());
        assert_equal(10000, key, "");
    }
}

void test() {
    testPutAndQuery();
    testIterator();
    testRemove();
    testReinsert();
    testConstValue();
    testBigData();
}

// test for matu

#include "matu.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int cmd = 0;

bool CheckFile(string fileName) {
    ifstream file(fileName, ios::binary | ios::in);
    if (!file.is_open())
        return false;
    file.seekg(0, ios::beg);
    char result[4] = { 0xc1, 0xc6, 0xf0, 0x1e };
    while (file.good()) {
        char buff[4];
        if (file.read(buff, sizeof(buff))) {
            if (strncmp(buff, result, sizeof(buff)) != 0) {
                return false;
            }
        }
        file.seekg(4092, ios::cur);
    }
    file.close();
    return true;
}

void testM1() {
    initial();

    for (int i = 0; i < 10000; i++) {
        vector<RowData> rowDatas;
        RowData rowData;
        rowData.id = i;
        rowData.name = string("ccg").append(to_string(i));
        rowData.number = string("20190603").append(to_string(i));
        rowData.sex = true;
        rowData.email = string("20190603@").append(to_string(i)).append(".com");
        rowDatas.push_back(rowData);
        insert(rowDatas);
    }

    if (!CheckFile("table.db")) {
        printf("wrong\n");
        return;
    }
    vector<RowData> result =
        query("select * from table where id > 500 and id <999");
    bool flag = true;
    for (int i = 501; i < 999; i++) {
        RowData rowData = result.front();
        result.erase(result.begin());
        if (rowData.id != i) {
            flag = false;
            break;
        }
        if (rowData.name.compare(string("ccg").append(to_string(i))) != 0) {
            flag = false;
            break;
        }
        if (rowData.number.compare(string("20190603").append(to_string(i))) != 0) {
            flag = false;
            break;
        }
        if (!rowData.sex) {
            flag = false;
            break;
        }
        if (rowData.email.compare(
            string("20190603@").append(to_string(i)).append(".com")) != 0) {
            flag = false;
            break;
        }
    }
    // del("delete from table where id > 2956 and id < 7854");
    if (flag == false) {
        printf("wrong\n");
    }
    else
        printf("yes\n");
}

void testM3() {
    update("update table set number = 20190603x where id<50 or id>9950");
    vector<RowData> rowDatas =
        query("select * from table where id<50 or id>9950");
    if (rowDatas.size() != 99) {
        printf("wrong\n");
        return;
    }
    for (vector<RowData>::iterator it = rowDatas.begin(); it != rowDatas.end();
        it++) {
        if ((*it).number.compare(string("20190603x")) != 0) {
            printf("wrong\n");
            return;
        }
    }
    if (!CheckFile("table.db")) {
        printf("wrong\n");
        return;
    }
    printf("yes\n");
    return;
}

void testM2() {
    int flag = true;
    del("delete from table where id > 2956 and id < 7854");
    vector<RowData> result =
        query("select * from table where id > 2900 and id <5000");
    for (int i = 2901; i < 2957; i++)
        // for (int i = 2991; i < 2957; i++)
    {
        //?:从2901 开始
        RowData rowData = result.front();
        result.erase(result.begin());
        if (rowData.id != i) {
            flag = false;
            break;
        }
        if (rowData.name.compare(string("ccg").append(to_string(i))) != 0) {
            flag = false;
            break;
        }
        if (rowData.number.compare(string("20190603").append(to_string(i))) != 0) {
            flag = false;
            break;
        }
        if (!rowData.sex) {
            flag = false;
            break;
        }
        if (rowData.email.compare(
            string("20190603@").append(to_string(i)).append(".com")) != 0) {
            flag = false;
            break;
        }
    }
    if (flag == false) {
        printf("wrong\n");
        return;
    }
    result = query("select * from table where id > 5000 and id < 7900");
    for (int i = 7854; i < 7900; i++)
        // for (int i = 7855; i < 7900; i++)
    {
        // ?: 从7854开始
        RowData rowData = result.front();
        result.erase(result.begin());
        if (rowData.id != i) {
            flag = false;
            break;
        }
        if (rowData.name.compare(string("ccg").append(to_string(i))) != 0) {
            flag = false;
            break;
        }
        if (rowData.number.compare(string("20190603").append(to_string(i))) != 0) {
            flag = false;
            break;
        }
        if (!rowData.sex) {
            flag = false;
            break;
        }
        if (rowData.email.compare(
            string("20190603@").append(to_string(i)).append(".com")) != 0) {
            flag = false;
            break;
        }
    }
    if (flag == false) {
        printf("wrong\n");
        return;
    }

    if (!CheckFile("table.db")) {
        printf("wrong\n");
        return;
    }
    printf("yes\n");
}

void test2() {
    initial();
    vector<RowData> rowDatas;
    for (int i = 0; i < 10000; i++) {
        RowData rowData;
        rowData.id = i;
        rowData.name = string("ccg").append(to_string(i));
        rowData.number = string("20190603").append(to_string(i));
        rowData.sex = true;
        rowData.email = string("20190603@").append(to_string(i)).append(".com");
        rowDatas.push_back(rowData);
    }
    insert(rowDatas);

    vector<RowData> result =
        query("select * from table where id <5000 and id>200");
    bool flag = true;
    for (int i = 201; i < 5000; i++) {
        RowData rowData = result[i - 201];
        if (rowData.id != i) {
            flag = false;
            break;
        }
        if (rowData.name.compare(string("ccg").append(to_string(i))) != 0) {
            flag = false;
            break;
        }
        if (rowData.number.compare(string("20190603").append(to_string(i))) != 0) {
            flag = false;
            break;
        }
        if (!rowData.sex) {
            flag = false;
            break;
        }
        if (rowData.email.compare(
            string("20190603@").append(to_string(i)).append(".com")) != 0) {
            flag = false;
            break;
        }
    }
    result = query("select * from table where id <500 or id>10000");

    // del("delete from table where id > 2956 and id < 7854");
    if (flag == false)
        printf("wrong\n");
    else
        printf("yes\n");
}
