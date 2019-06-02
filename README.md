# Pedar Programming Language
version 1.0.0 alpha 1

![Gopher image](pedar-small.png)

# Welcome to Pedar
Pedar is an open source programming language that makes it easy to build simple and efficient software. It has a clean and modern syntax.
This is the official repository for the Pedar Programming Language standard library and language spec.

# How to contribute
To contribute in this repo, please open a pull request from your fork of this repository. We do have to ask you to sign the Pedar before we can merge any of your work, to protect its open source nature.

For more information on building and developing the core of Pedar.

```dart
import "io.p";

main(){
    print("Hello World!\n");

    f = open("help.chw", 'r');
    print("open file by fd %n\n", f.fd);

    buf = f.read();
    print("%s\n", buf);

    print("tell %n\n", f.tell());
    print("cwd %s\n", cwd());
    print("rename %n\n", rename("help","help.chw"));

    f.close();

    t = tick();
    sum = 0;
    i = 0;
    while(i < 1000000){
        sum = sum + i;
        i = i + 1;
    }

    fmt = format("%.2n in %.8n sec\n", sum, (tick() - t) / 1000000);
    print(fmt);
    print("size of fmt %.2n\n", sizeof(fmt));
    print("type of fmt %s\n", typeof(fmt));

    list = [0,10,20,30,40,50,60,70,80,90];
    delete(list, 1, 2, 3, 4, 5, 6, 7);
    print("%s %n\n", list, count(list));

    print("what's your name?\n");
    input = gets(100);
    print("%s", input);

    getc();
}
```

## How to define a class ?
```dart
class_name {
    this(){
        return this;
    }

    function(a){
        return a;
    }
}
```

or

```dart
class_name {
    class_name(){
        return this;
    }

    function(a){
        return a;
    }
}
```

## How to use of class ?
```dart
a = class_name(parameters);
b = a.function(parameters);
```

## How to create a list(tuple) ?
```dart
a = [value1, value2, value3 , ...];
cnt = count(a);
delete(a, index1, index2, index3, ...);
```

# Inheritance
The transfer of the characteristics of a class to other classes that are derived from it.

```dart
parent{
    this(){

    }
    fn(a, b){
        return a + b;
    }
}

child : parent{
    this(){

    }
    fn2(a, b){
        return a - b;
    }
}
```

# Keywords

## import
```dart
import "modulus_path";
```

or
```dart
import 'modulus_path';
```

## this
'this' object refer to class contain it, for example
```dart
class_name {
    this(){
        return this;
    }
}
```
'this' refer to 'class_name'

## super
'super' object refer to first parent class contain it, for example
```dart
super_class_name {
    this(){
        return this;
    }

    class_name {
        this(a){
            super(a);
            return this;
        }
    }
}
```
'super' refer to 'super_class_name'

## while
using 'while' for loop, for example
```dart
while([condition]){
    [statement]
}
```

## if
using 'if' for condition, for example
```dart
if([condition]){
    [statement]
}
```

## else
using 'if' for condition, for example
```dart
if([condition]){
    [statement]
}
else if([condition]){
    [statement]
} else {
    [statement]
}
```

## delete
using 'delete' for delete a variable or, remove items of a list, for example
```dart
list = [1,2,3];
delete(list, 0, 1);
```

## count
using 'count' for count items in list, for example
```dart
list = [1,2,3];
n = count(list);
```
## null
## continue
## break
## return

## sizeof
using 'sizeof' for get size of variable, for example
```dart
list = [1,2,3];
n = sizeof(list);
```

## typeof
using 'typeof' for get type of variable, for example
```dart
list = [1,2,3];
n = typeof(list);
```

## print
using 'print' for write a string or number in console, for example
```dart
print("string is %s and number is %.5n \n", 'hello world!', 3.14);
```

## format
using 'format' for create a string formated, similar print format, for example
```dart
fmt = format("string is %s and number is %.5n \n", 'hello world!', 3.14);
```

## ref
using 'ref' for create a reference variable, for example;
```dart
class_name {
    this(ref a){
        a = 3;
        return this;
    }
}
```
# TIME && TICK
## time
This function returns the time since 00:00:00 UTC, January 1, 1970 (Unix timestamp) in seconds.
```dart
t = time();
```
## tick
What is Tick?
Time intervals are floating-point numbers in units of seconds. Particular instants in time are expressed in seconds since 12:00am, January 1, 1970(epoch).

```dart
t = tick();
```

# Working with files
## __open__
fd __open__(path, flag);

flag: How you like to use
0: read only,
1: write only,
2: read and write,
3: create file if it doesn’t exist,
4: prevent creation if it already exists

Find existing file on disk
Create file table entry
Set first unused file descriptor to point to file table entry
Return file descriptor used, -1 upon failure

## __close__
Parameter
fd :file descriptor

Return
0 on success.
-1 on error.

How it works in the OS
Destroy file table entry referenced by element fd of file descriptor table
– As long as no other process is pointing to it!
Set element fd of file descriptor table to null

## __read__
From the file indicated by the file descriptor fd, the __read__() function reads cnt bytes of input into the memory area indicated by buf. A successful read() updates the access time for the file.

Syntax
size __read__ (fd, buf, cnt);  

Parameters
fd: file descripter
buf: buffer to read data from
cnt: length of buffer

Returns: How many bytes were actually read
return Number of bytes read on success
return 0 on reaching end of file
return -1 on error
return -1 on signal interrupt

## __write__
Writes cnt bytes from buf to the file or socket associated with fd. cnt should not be greater than long size. If cnt is zero, __write__() simply returns 0 without attempting any other action.

size __write__ (fd, buf, cnt);

Parameters
fd: file descripter
buf: buffer to write data to
cnt: length of buffer

Returns: How many bytes were actually written
return Number of bytes written on success
return 0 on reaching end of file
return -1 on error
return -1 on signal interrupt

## __seek__
seek file to cnt
__seek__(fd, cnt, flag);

flags :
0 : from first,
1 : from current,
2 : from end

## rename
rename a file
rename(old_path, new_path);

## cwd
return current dirrectory path
cwd();

## chdir
change current directory path

# Build Instructions
make
make test

# Install && Using
make install
pedar test1.p

# Library
+ 'io.p'
    - open(path, flag)
        flag : 'r' readable, 'w' writeable , 'rw' readable and writeable, 'c' create and open

    - read()
    - read(cnt)
        cnt: length of buffer

    - write(buf)
        buf: buffer to write data

    - seek(cnt, flag)
        cnt: length of seek
        flag : 'b' begin, 'c' current, 'e' end

    - close()
    - tell()
# contact us
    We are waiting for your email.
        Email: yassersajjadi@gmail.com
