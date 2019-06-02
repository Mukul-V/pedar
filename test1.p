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
