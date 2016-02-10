// jsoncons_test.cpp : Defines the entry point for the console application.
//

#include <fstream>
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/json_query.hpp"

using jsoncons::json;
using jsoncons::wjson;
using jsoncons::jsonpath::json_query;

namespace readme_examples {

void example1()
{
    // Construct a book object
    json book1;

    book1["category"] = "Fiction";
    book1["title"] = "A Wild Sheep Chase: A Novel";
    book1["author"] = "Haruki Murakami";
    book1["date"] = "2002-04-09";
    book1["price"] = 9.01;
    book1["isbn"] = "037571894X";  

    // Construct another using the member function set
    json book2;

    book2.set("category", "History");
    book2.set("title", "Charlie Wilson's War");
    book2.set("author", "George Crile");
    book2.set("date", "2007-11-06");
    book2.set("price", 10.50);
    book2.set("isbn", "0802143415");  

    // Use set again, but more efficiently
    json book3;

    // Reserve memory, to avoid reallocations
    book3.reserve(6);

    // Insert in name alphabetical order
    // Give set a hint where to insert the next member
    json::object_iterator hint;
    hint = book3.set(book3.members().begin(),"author", "Haruki Murakami");
    hint = book3.set(hint, "category", "Fiction");
    hint = book3.set(hint, "date", "2006-01-03");
    hint = book3.set(hint, "isbn", "1400079276");  
    hint = book3.set(hint, "price", 13.45);
    hint = book3.set(hint, "title", "Kafka on the Shore");

    // Construct a fourth from a string
#if !defined(JSONCONS_NO_RAW_STRING_LITERALS)
    json book4 = json::parse(R"(
    {
        "category" : "Fiction",
        "title" : "Pulp",
        "author" : "Charles Bukowski",
        "date" : "2004-07-08",
        "price" : 22.48,
        "isbn" : "1852272007"  
    }
    )");
#else
    json book4;
#endif

    // Construct a booklist array
    json booklist = json::array();

    // For efficiency, reserve memory, to avoid reallocations
    booklist.reserve(4);

    // For efficency, tell jsoncons to move the contents 
    // of the four book objects into the array
    booklist.add(std::move(book1));    
    booklist.add(std::move(book2));    

    // Add the third one to the front
    auto where = booklist.add(booklist.elements().begin(),std::move(book3));
    
    // Add the last one immediately after
    booklist.add(where+1,std::move(book4));    

	// See what's left of book1, 2, 3 and 4 (expect nulls)
	std::cout << book1 << "," << book2 << "," << book3 << "," << book4 << std::endl;

    //Loop through the booklist elements using a range-based for loop    
#if !defined(JSONCONS_NO_FOR_RANGE)
    for(auto book : booklist.elements())
    {
        std::cout << book["title"].as<std::string>()
                  << ","
                  << book["price"].as<double>() << std::endl;
    }
#endif

    // The second book
    json& book = booklist[1];

    //Loop through the book members using a range-based for loop    
#if !defined(JSONCONS_NO_FOR_RANGE)
    for(auto member : book.members())
    {
        std::cout << member.name()
                  << ","
                  << member.value() << std::endl;
    }
#endif

    auto it = book.find("author");
    if (it != book.members().end())
    {
        // member "author" found
    }

    if (book.count("author") > 0)
    {
        // book has member "author"
    }

    book.get("author", "author unknown").as<std::string>();
    // Returns author if found, otherwise "author unknown"

    try
    {
        book["ratings"].as<std::string>();
    }
    catch (const std::out_of_range&)
    {
        // member "ratings" not found
    }

    // Add ratings
    book["ratings"]["*****"] = 4;
    book["ratings"]["*"] = 1;

    // Delete one-star ratings
    book["ratings"].erase("*");

    // Serialize the booklist to a file
    std::ofstream os("booklist.json");
    os << pretty_print(booklist);
}

void example2()
{
    // Deserialize the booklist
    std::ifstream is("booklist.json");
    json booklist;
    is >> booklist;

    // Use a JsonPath expression to find 
      
    // (1) The authors of books that cost less than $12
    json result = json_query(booklist, "$[*][?(@.price < 12)].author");
    std::cout << "(1) " << result << std::endl;

    // (2) The number of books
    result = json_query(booklist, "$.length");
    std::cout << "(2) " << result << std::endl;

    // (3) The third book
    result = json_query(booklist, "$[2]");
    std::cout << "(3) " << std::endl << pretty_print(result) << std::endl;

    // (4) The authors of books that were published in 2004
    result = json_query(booklist, "$[*][?(@.date =~ /2004.*?/)].author");
    std::cout << "(4) " << result << std::endl;

    // (5) The titles of all books that have ratings
    result = json_query(booklist, "$[*][?(@.ratings)].title");
    std::cout << "(5) " << result << std::endl;
}

void example3()
{
	// Construct a book object
	wjson book1;

	book1[L"category"] = L"Fiction";
	book1[L"title"] = L"A Wild Sheep Chase: A Novel";
	book1[L"author"] = L"Haruki Murakami";
	book1[L"date"] = L"2002-04-09";
	book1[L"price"] = 9.01;
	book1[L"isbn"] = L"037571894X";

	// Construct another using the member function set
	wjson book2;

	book2.set(L"category", L"History");
	book2.set(L"title", L"Charlie Wilson's War");
	book2.set(L"author", L"George Crile");
	book2.set(L"date", L"2007-11-06");
	book2.set(L"price", 10.50);
	book2.set(L"isbn", L"0802143415");

    // Use set again, but more efficiently
    wjson book3;

    // Reserve memory, to avoid reallocations
    book3.reserve(6);

    // Insert in name alphabetical order
    // Give set a hint where to insert the next member
    wjson::object_iterator hint;
    hint = book3.set(book3.members().begin(), L"author", L"Haruki Murakami");
    hint = book3.set(hint, L"category", L"Fiction");
    hint = book3.set(hint, L"date", L"2006-01-03");
    hint = book3.set(hint, L"isbn", L"1400079276");
    hint = book3.set(hint, L"price", 13.45);
    hint = book3.set(hint, L"title", L"Kafka on the Shore");

	// Construct a fourth from a string

#if !defined(JSONCONS_NO_RAW_STRING_LITERALS)
	wjson book4 = wjson::parse(LR"(
    {
        "category" : "Fiction",
        "title" : "Pulp",
        "author" : "Charles Bukowski",
        "date" : "2004-07-08",
        "price" : 22.48,
        "isbn" : "1852272007"  
    }
    )");
#else
    wjson book4;
#endif
    // Construct a booklist array
    wjson booklist = wjson::array();

    // For efficiency, reserve memory, to avoid reallocations
    booklist.reserve(4);

    // For efficency, tell jsoncons to move the contents 
    // of the four book objects into the array
    booklist.add(std::move(book1));
    booklist.add(std::move(book2));

    // Add the third one to the front
    auto where = booklist.add(booklist.elements().begin(),std::move(book3));

    // Add the last one immediately after
    booklist.add(where+1,std::move(book4));    

    // See what's left of book1, 2, 3 and 4 (expect nulls)
    std::wcout << book1 << L"," << book2 << L"," << book3 << L"," << book4 << std::endl;

    //Loop through the booklist elements using a range-based for loop    
#if !defined(JSONCONS_NO_FOR_RANGE)
    for (auto book : booklist.elements())
    {
        std::wcout << book[L"title"].as<std::wstring>()
          	       << L","
        	       << book[L"price"].as<double>() << std::endl;
    }
#endif

    // The second book
    wjson& book = booklist[1];

    //Loop through the book members using a range-based for loop    
#if !defined(JSONCONS_NO_FOR_RANGE)
    for(auto member : book.members())
    {
        std::wcout << member.name()
                   << L","
                   << member.value() << std::endl;
    }
#endif

    auto it = book.find(L"author");
    if (it != book.members().end())
    {
        // book has member "author"
    }

    if (book.count(L"author") > 0)
    {
        // book has member "author"
    }

    book.get(L"author", L"author unknown").as<std::wstring>();
    // Returns author if found, otherwise "author unknown"

    try
    {
        book[L"ratings"].as<std::wstring>();
    }
    catch (const std::out_of_range&)
    {
        // member "ratings" not found
    }

    // Add ratings
    book[L"ratings"][L"*****"] = 4;
    book[L"ratings"][L"*"] = 2;

    // Delete one-star ratings
    book[L"ratings"].erase(L"*");

    // Serialize the booklist to a file
    std::wofstream os(L"booklist2.json");
    os << pretty_print(booklist);
}

void example4()
{
    // Deserialize the booklist
    std::wifstream is("booklist2.json");
    wjson booklist;
    is >> booklist;

    // Use a JsonPath expression to find 
    //  
    // (1) The authors of books that cost less than $12
    wjson result = json_query(booklist, L"$[*][?(@.price < 12)].author");
    std::wcout << L"(1) " << result << std::endl;

    // (2) The number of books
    result = json_query(booklist, L"$.length");
    std::wcout << L"(2) " << result << std::endl;

    // (3) The third book
    result = json_query(booklist, L"$[2]");
    std::wcout << L"(3) " << std::endl << pretty_print(result) << std::endl;

    // (4) The authors of books that were published in 2004
    result = json_query(booklist, L"$[*][?(@.date =~ /2004.*?/)].author");
    std::wcout << L"(4) " << result << std::endl;

    // (5) The titles of all books that have ratings
    result = json_query(booklist, L"$[*][?(@.ratings)].title");
    std::wcout << L"(5) " << result << std::endl;
}

}

void run_readme_examples()
{
    readme_examples::example1();
    readme_examples::example2();
    readme_examples::example3();
    readme_examples::example4();
}

