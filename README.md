# The Pedar Programming Language
GNU General Public License v3.0
(version 1.0.0 alpha 1)

![Gopher image](pedar-small.png)

# How to contribute
To contribute in this repo, please open a pull request from your fork of this repository. We do have to ask you to sign the Pedar before we can merge any of your work, to protect its open source nature.

For more information on building and developing the core of Pedar.

```dart
import ('io', 'time');

main(){
    print("Hello World!\n");

    print("string is %s and number is %.5n \n", 'hello world!', 3.14, 'format %n\n', 2);

    file = io.file;
    keyboard = io.keyboard;

    f = file.open("help.chw", 'r');

    print("open file by fd %n\n", f.fd);

    buf = f.read(f.length());
    print("%s\n", buf);

    print("tell %n\n", f.tell());

    print("cwd %s\n", file.cwd());
    print("rename %n\n", file.rename("help","help.chw"));

    f.close();

    t = time.tick();
    sum = 0;
    i = 0;
    while(i < 1000000){
        sum = sum + i;
        i = i + 1;
    }

    fmt = format("%.2n in %.8n sec\n", sum, (time.tick() - t) / 1000000);
    print(fmt);
    print("size of fmt %.2n\n", sizeof(fmt));
    print("type of fmt %s\n", typeof(fmt));

    list = [0,10,20,30,40,50,60,70,80,90];
    insert(list, 1, 300);
    delete(list, 1, 2, 3, 4, 5, 6, 7);

    print("%s %n\n", list, count(list));

    print("what's your name?\n");
    input = keyboard.gets(100);
    print("%s", input);

    keyboard.get();
}
```

+ A Pedar program basically consists of the following parts
    - Import Packages
    - Category
    - Variables
    - Statements and Expressions
    - Comments

# The next goal and task

```dart
/* Category is considered a class or function. 
The variable 'pedar' contains a category. When calling a category (input or function), the input parameters are sent to the variable containing the category, so you can call them from the variable.
*/
/* 
متغیر 'pedar' 
متغیری شامل یک کتگوری است. در هنگام فراخوانی کتگوری(کلاس یا تابع) پارامترهای ورودی به متغیر حاوی کتگوری فرستاده می شوند بنابراین شما میتوانید آنها را از متغیر فراخوانی کنید
*/
pedar = {
    /* Parameters 1,2 and 3 of the 'pedar' category are received as input */
    parameter1 <- pedar;
    parameter2 <- pedar;
    parameter3 <- pedar;

    fun = {
        parameter4 <- pedar;
        fun <- ((parameter5 <- fun) + parameter1 + parameter2 + parameter3 + parameter4);
        //or return ((parameter5 <- fun) + parameter1 + parameter2 + parameter3 + parameter4);
    }
    
    return this;
    // this -> pedar;
}

a = pedar(value1,value2,value3);
// pedar class with 3 parameters not included the fun function

b = pedar(value1,value2,value3,value4);
// pedar class with 4 parameters containing the fun function

c = b.fun(value1);

```

# The Author
    Pedar was originally developed by Yasser Sajjadi (Ys) in 2019.
        Email: yassersajjadi@gmail.com

# License
GNU General Public License v3.0
