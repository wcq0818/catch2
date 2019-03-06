#include "catch.hpp"
#include <iostream>
#include <boost/timer.hpp>
#include <queue>
#include <list>
#include <forward_list>

using namespace std;
using namespace boost;

TEST_CASE("vector", "[vector][.hide]")
{
	int length = 1000000;
	vector<int> datas;
	timer t;
	{
		t.restart();
		for (size_t i = 0; i < length; i++)
		{
			datas.push_back(i);
		}
		cout << "vector push_back time : " << t.elapsed() << "s" << endl;

		t.restart();
		while (datas.empty() == false)
		{
			datas.pop_back();
			//cout << "front : " << datas.front() << ", back : " << datas.back() << endl;
		}
		cout << "vector pop_back time : " << t.elapsed() << "s" << endl;
	}
	cout << endl;
}

TEST_CASE("queue", "[queue][.hide]")
{
	int length = 10000;
	queue<int> datas;
	timer t;
	{
		t.restart();
		for (size_t i = 0; i < length; i++)
		{
			datas.push(i);
			//cout << "front : " << datas.front() << ", back : " << datas.back() << endl;
		}
		cout << "queue push time : " << t.elapsed() << "s" << endl;

		t.restart();
		while (datas.empty() == false)
		{
			//cout << "front : " << datas.front() << ", back : " << datas.back() << endl;
			datas.pop();
		}
		cout << "queue pop time : " << t.elapsed() << "s" << endl;
	}
	cout << endl;
}

TEST_CASE("deque", "[deque][.hide]")
{
	int length = 10000;
	deque<int> datas;
	timer t;
	{
		t.restart();
		for (size_t i = 0; i < length; i++)
		{
			datas.push_back(i);
			//cout << "front : " << datas.front() << ", back : " << datas.back() << endl;
		}
		cout << "deque push_back time : " << t.elapsed() << "s" << endl;

		t.restart();
		while (datas.empty() == false)
		{
			datas.pop_back();
			//cout << "front : " << datas.front() << ", back : " << datas.back() << endl;
		}
		cout << "deque pop_back time : " << t.elapsed() << "s" << endl;

		t.restart();
		for (size_t i = 0; i < length; i++)
		{
			datas.push_front(i);
			//cout << "front : " << datas.front() << ", back : " << datas.back() << endl;
		}
		cout << "deque push_front time : " << t.elapsed() << "s" << endl;

		t.restart();
		while (datas.empty() == false)
		{
			//cout << "front : " << datas.front() << ", back : " << datas.back() << endl;
			datas.pop_front();
		}
		cout << "deque pop_front time : " << t.elapsed() << "s" << endl;
	}
	cout << endl;
}

TEST_CASE("list", "[list][.hide]")
{
	int length = 1000000;
	list<int> datas;
	timer t;
	{
		t.restart();
		for (size_t i = 0; i < length; i++)
		{
			datas.push_back(i);
		}
		cout << "list push_back time : " << t.elapsed() << "s" << endl;

		t.restart();
		cout << "front : " << datas.front() << ", back : " << datas.back() << endl;
		cout << "list time : " << t.elapsed() << "s" << endl;

		t.restart();
		while (datas.empty() == false)
		{
			datas.pop_back();
		}
		cout << "list pop_back time : " << t.elapsed() << "s" << endl;

		t.restart();
		for (size_t i = 0; i < length; i++)
		{
			datas.push_front(i);
		}
		cout << "list push_front time : " << t.elapsed() << "s" << endl;

		t.restart();
		cout << "front : " << datas.front() << ", back : " << datas.back() << endl;
		cout << "list time : " << t.elapsed() << "s" << endl;

		t.restart();
		while (datas.empty() == false)
		{
			datas.pop_front();
		}
		cout << "list pop_front time : " << t.elapsed() << "s" << endl;
	}
	cout << endl;
}

TEST_CASE("forward_list", "[forward_list][hide]")
{
	int length = 10000;
	forward_list<int> datas;
	timer t;
	{
		t.restart();
		for (size_t i = 0; i < length; i++)
		{
			datas.push_front(i);
			cout << "front : " << datas.front() << endl;
		}
		cout << "forward_list push_front time : " << t.elapsed() << "s" << endl;

		t.restart();
		while (datas.empty() == false)
		{
			cout << "front : " << datas.front() << endl;
			datas.pop_front();
		}
		cout << "forward_list pop_front time : " << t.elapsed() << "s" << endl;
	}
	cout << endl;
}

