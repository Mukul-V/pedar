open {
    fd = 0;
    mode = 'r';
    path = null;

    this(str, md){
        if(typeof(md) != 'DATA'){
            print('error, mode is a string variable\nr:readable, w:writable, rw:readable and writable, c:create and open\n');
            return null;
        }

        mode = md;

        open_mode = 0;
        if(mode == 'w'){
            open_mode = 1;
        }
        else if(mode == 'rw'){
            open_mode = 2;
        }
        else if(mode == 'c'){
            open_mode = 64;
        }

        fd = __open__(str, open_mode);
        path = str;
        return this;
    }

    close(){
        if(fd > 0){
            ret = __close__(fd);
            fd = 0;
            return ret;
        }
        return -1;
    }

    seek(index, flag){
        if(fd > 0){
            seek_mode = 0;
            if(flag == 'c'){
                seek_mode = 1;
            }
            else if(flag == 'e'){
                seek_mode = 2;
            }
            return __seek__(fd, index, seek_mode);
        }
        return -1;
    }

    tell(){
        if(fd > 0){
            return __seek__(fd, 0, 1);
        }
        return -1;
    }

    length(){
        if(fd > 0){
            len = __seek__(fd, 0, 2);
            __seek__(fd, 0, 0);
            return len;
        }
        return -1;
    }

    read(len){
        if(fd > 0){
            if(__read__(fd, buf, len) < 0){
                return null;
            }
            return buf;
        }
        return null;
    }

    read(){
        if(fd > 0){
            len = __seek__(fd, 0, 2);
            __seek__(fd, 0, 0);
            if(__read__(fd, buf, len) < 0){
                return null;
            }
            return buf;
        }
        return null;
    }

    write(buf){
        if(fd > 0){
            return __write__(fd, buf, count(buf));
        }
        return -1;
    }

}
