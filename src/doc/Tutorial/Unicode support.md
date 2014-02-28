## Narrow character support for UTF8 encoding

### Reading escaped unicode into utf8 encodings and writing back escaped unicode

    string inputStr("[\"\\u007F\\u07FF\\u0800\"]");
    std::cout << "Input:    " << inputStr << std::endl;

    json arr = json::parse_string(inputStr);
    std::string s = arr[0].as<string>();
    std::cout << "Hex dump: [";
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (i != 0)
            std::cout << " ";
        unsigned int u(s[i] >= 0 ? s[i] : 256 + s[i] );
        std::cout << "0x"  << std::hex<< std::setfill('0') << std::setw(2) << u;
    }
    std::cout << "]" << std::endl;
    
    std::ostringstream os;
    output_format format;
    format.escape_all_non_ascii(true);
    arr.to_stream(os,format);
    std::string outputStr = os.str();
    std::cout << "Output:   " << os.str() << std::endl;

    json arr2 = json::parse_string(outputStr);
    std::string s2 = arr2[0].as<string>();
    std::cout << "Hex dump: [";
    for (size_t i = 0; i < s2.size(); ++i)
    {
        if (i != 0)
            std::cout << " ";
        unsigned int u(s2[i] >= 0 ? s2[i] : 256 + s2[i] );
        std::cout << "0x"  << std::hex<< std::setfill('0') << std::setw(2) << u;
    }
    std::cout << "]" << std::endl;

The output is

    Input:    ["\u007F\u07FF\u0800"]
    Hex dump: [0x7f 0xdf 0xbf 0xe0 0xa0 0x80]
    Output:   ["\u007F\u07FF\u0800"]
    Hex dump: [0x7f 0xdf 0xbf 0xe0 0xa0 0x80]

Since the escaped unicode consists of a control character (0x7f) and non-ascii, we get back the same text as what we started with.

### Reading escaped unicode into utf8 encodings and writing back escaped unicode (with continuations)

    string input = "[\"\\u8A73\\u7D30\\u95B2\\u89A7\\uD800\\uDC01\\u4E00\"]";
    json value = json::parse_string(input);
    output_format format;
    format.escape_all_non_ascii(true);
    string output = value.to_string(format);

    std::cout << "Input:" << std::endl;
    std::cout << input << std::endl;
    std::cout << std::endl;
    std::cout << "Output:" << std::endl;
    std::cout << output << std::endl;

Since all of the escaped unicode is non-ascii, we get back the same text as what we started with.

    Input:
    ["\u8A73\u7D30\u95B2\u89A7\uD800\uDC01\u4E00"]

    Output:
    ["\u8A73\u7D30\u95B2\u89A7\uD800\uDC01\u4E00"]

## Wide character support for UTF16 and UTF32 encodings

jsoncons supports wide character strings and streams with `wjson` and `wjson_reader`. It assumes `UTF16` encoding if `wchar_t` has size 2 and `UTF32` encoding if `wchar_t` has size 4.

    using jsoncons::wjson;

    wjson root;
    root[L"field1"] = L"test";
    root[L"field2"] = 3.9;
    root[L"field3"] = true;
    std::wcout << root << L"\n";

The output is

    {"field1":"test","field2":3.9,"field3":true}

### Escaped unicode

    wstring input = L"[\"\\u007F\\u07FF\\u0800\"]";
    std::wistringstream is(input);

    wjson val = wjson::parse(is);

    wstring s = val[0].as<wstring>();
    std::cout << "length=" << s.length() << std::endl;
    std::cout << "Hex dump: [";
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (i != 0)
            std::cout << " ";
        uint32_t u(s[i] >= 0 ? s[i] : 256 + s[i] );
        std::cout << "0x"  << std::hex<< std::setfill('0') << std::setw(2) << u;
    }
    std::cout << "]" << std::endl;

    std::wofstream os("output/xxx.txt");
    os.imbue(std::locale(os.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
    
    woutput_format format;
    format.escape_all_non_ascii(true);

    os << pretty_print(val,format) << L"\n";

The output is

    length=3
    Hex dump: [0x7f 0x7ff 0x800]

and the file `xxx.txt` contains
    
    ["\u007F\u07FF\u0800"]    
