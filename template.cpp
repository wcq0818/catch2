#include "catch.hpp"
#include <limits>
#include <iostream>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant.hpp>
#include <optional>
#include <algorithm>

#include "boost/multi_index_container.hpp"//boost::multi_index::multi_index_container
#include "boost/multi_index/ordered_index.hpp"//insert
#include "boost/multi_index/member.hpp"



TEST_CASE("optional", "[optional]")
{
	std::optional<int> op;
	if (op)
		std::cout << *op << std::endl;

	std::optional<int> op1 = 1;
	if (op1)
		std::cout << *op1 <<std:: endl;
}


template <typename... T>
using testType = boost::variant<T...>;

template <typename T>
class Adapter
{
public:
	T value_  = 0;

public:
	void fun() const
	{
		std::cout << value_ << std::endl;
	}
};

template <typename T = Adapter>
class testClass_visitor : public boost::static_visitor<void>
{
public:
	template <typename T>
	void operator()(T& value) const
	{
		adapter_.fun();
	}

public:
	T adapter_;
};

template <typename T>
using normalClass = testClass_visitor<Adapter<T>>;

TEST_CASE("template", "[template][.hide]")
{
	normalClass<int> visit;

	testType<int, double> value;

 	boost::apply_visitor(visit, value);
}

TEST_CASE("distance", "[distance][.hide]")
{
	std::vector<int> v{ 3, 1, 4 };
	int len = std::distance(v.begin(), v.end());
	std::cout << "distance(first, last) = " << std::distance(v.begin(), v.end()) << '\n' << "distance(last, first) = " << std::distance(v.end(), v.begin()) << '\n';
	//the behavior is undefined (until C++11)
}

struct Employee
{
	Employee(int id_, const std::string& name_) :
		id(id_),
		name(name_) {
	}

	int id;//编号
	std::string name;//姓名

					 //default compare by id
	bool operator<(const Employee& employee) const
	{
		return id < employee.id;
	}

	friend std::ostream& operator<<(std::ostream& out, const Employee& employee)
	{
		out << employee.id << "\t" << employee.name << std::endl;
		return out;
	}
};

typedef boost::multi_index::multi_index_container<
	Employee,
	boost::multi_index::indexed_by<
		boost::multi_index::ordered_unique<
			boost::multi_index::identity<Employee> >,
		boost::multi_index::ordered_non_unique<
			boost::multi_index::member<Employee, std::string, &Employee::name> >
	>
> EmployeeSet;

TEST_CASE("multi_index::multi_index_container", "[multi_index::multi_index_container][.hide]")
{
	EmployeeSet employees;

	employees.insert({ 5, "Jeff Dean" });
	employees.insert({ 1, "Google" });
	employees.insert({ 3, "Bidu" });
	employees.insert({ 2, "Markus Heule" });
	employees.insert({ 4, "Vlad Losev" });

	//1       Google
	//2       Markus Heule
	//3       Bidu
	//4       Vlad Losev
	//5       Jeff Dean
	std::copy(
		employees.begin(), //equal to employees.get<0>()
		employees.end(),
		std::ostream_iterator<Employee>(std::cout));

	std::cout << std::endl;

	const EmployeeSet::nth_index<1>::type& name_index = employees.get<1>();
	std::copy(
		name_index.begin(),
		name_index.end(),
		std::ostream_iterator<Employee>(std::cout));
}