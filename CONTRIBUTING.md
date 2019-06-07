## How to define a class ?
```dart
class_name {
    this(){
        return this;
    }

    fn_name(parameters){
        return null;
    }
}
```

or

```dart
class_name {
    class_name(){
        return this;
    }

    fn_name(parameters){
        return null;
    }
}
```

## How to use a class ?
```dart
a = class_name(parameters);
b = a.fn_name(parameters);
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
parent {
    this(){
        return this;
    }

    fn(a, b){
        return a + b;
    }
}

child : parent {
    this(){
        return this;
    }

    fn2(a, b){
        return a - b;
    }
}
```

# Keywords

## import
```dart
import ('library-path', ...);
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
*'this' refer to 'class_name'

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
*'super' refer to 'super_class_name'

## while
using 'while' for loop, for example
```dart
while(condition){
    statement
}
```

## if
using 'if' for condition, for example
```dart
if(condition){
    statement
}
```

## else
using 'if' for condition, for example
```dart
if(condition){
    statement
}
else if(condition){
    statement
} else {
    statement
}
```

## delete
using 'delete' for delete a variable or, remove items of a list, for example
```dart
list = [1,2,3];
delete(list, 0, 1);
```
## insert
using 'insert' for insert value in a list, for example;
```dart
insert(list, index, values ...);
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
print("string is %s and number is %.5n \n", 'hello world!', 3.14, 'format %n\n', 2);
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

# Build Instructions
    make
    make test

# Install and use
    make install
    pedar test1.p

# Library
+ io
    - io.file
        - open(path, flag)
            * flag : 'r' readable, 'w' writeable , 'rw' readable and writeable, 'c' create and open

        - read(cnt)
            * cnt: length of buffer

        - write(buf)
            * buf: buffer to write data

        - seek(cnt, flag)
            * cnt: length of seek
            * flag : 'b' begin, 'c' current, 'e' end

        - close()
        - tell()
        - cwd()
        - chdir(path)
        - rename(old_name, new_name)
        - walk(path)
            * return list of files in path

    - io.keyboard
        - gets()
        - get()

+ time
    - tick()
    - get()
