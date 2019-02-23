// IoTPolicy.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>

using namespace eosio;


class[[eosio::contract]] addressbook : public eosio::contract{

public:
  using contract::contract;

  addressbook(name receiver, name code,  datastream<const char*> ds) : contract(receiver, code, ds) {}

  [[eosio::action]]
  void upsert(name user, std::string first_name, std::string last_name, uint64_t age, std::string street, std::string city, std::string state) {
	require_auth(user);

	address_index addresses(_code, _code.value);

	auto iterator = addresses.find(user.value);
	if (iterator == addresses.end())
	{
	  addresses.emplace(user, [&](auto& row) {
	   row.key = user;
	   row.first_name = first_name;
	   row.last_name = last_name;
	   row.age = age;
	   row.street = street;
	   row.city = city;
	   row.state = state;
	   send_summary(user, " successfully emplaced record to addressbook");
	   increment_counter(user, "emplace");
	  });
	}
	else {
	  std::string changes;
	  addresses.modify(iterator, user, [&](auto& row) {

		if (row.first_name != first_name) {
		  row.first_name = first_name;
		  changes += "first name ";
		}

		if (row.last_name != last_name) {
		  row.last_name = last_name;
		  changes += "last name ";
		}

		if (row.age != age) {
		  row.age = age;
		  changes += "age ";
		}

		if (row.street != street) {
		  row.street = street;
		  changes += "street ";
		}

		if (row.city != city) {
		  row.city = city;
		  changes += "city ";
		}

		if (row.state != state) {
		  row.state = state;
		  changes += "state ";
		}
	  });

	  if (changes.length() > 0) {
		send_summary(user, " successfully modified record in addressbook. Fields changed: " + changes);
		increment_counter(user, "modify");
	  }
 else {
send_summary(user, " called upsert, but request resulted in no changes.");
}
}
}

[[eosio::action]]
void erase(name user) {
  require_auth(user);
  address_index addresses(_code, _code.value);
  auto iterator = addresses.find(user.value);
  eosio_assert(iterator != addresses.end(), "Record does not exist");
  addresses.erase(iterator);
  send_summary(user, " successfully erased record from addressbook");
  increment_counter(user, "erase");
}

[[eosio::action]]
void notify(name user, std::string msg) {
  require_auth(get_self());
  require_recipient(user);
}

private:

  struct[[eosio::table]] person {
	name key;
	std::string first_name;
	std::string last_name;
	uint64_t age;
	std::string street;
	std::string city;
	std::string state;
	uint64_t primary_key() const { return key.value; }
	uint64_t get_secondary_1() const { return age; }
  };

  void send_summary(name user, std::string message) {
	action(
	  permission_level{get_self(),"active"_n},
	  get_self(),
	  "notify"_n,
	  std::make_tuple(user, name{user}.to_string() + message)
	).send();
  };

  void increment_counter(name user, std::string type) {

	action counter = action(
	  permission_level{get_self(),"active"_n},
	  "abcounter"_n,
	  "count"_n,
	  std::make_tuple(user, type)
	);

	counter.send();
  }

  typedef eosio::multi_index<"people"_n, person,
	indexed_by<"byage"_n, const_mem_fun<person, uint64_t, &person::get_secondary_1>>
  > address_index;
};

EOSIO_DISPATCH(addressbook, (upsert)(notify)(erase))

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
