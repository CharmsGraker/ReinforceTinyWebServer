#Attention
this project was forked from TinyWebServer,<br/></br>
it was not code by me.<br/></br> I'm trying to improve it.
I'm a noob, so it just for learning myself.

# Update
### 2022.1.4
convert code, use template to construct Router.<br/>
compile become little slowly opz.However it is very flexible now.<br/>
use hashmap to accept url parameter only exist `http_conn` in viewer can make sense.<br/>
in all,if dont want use some data when connection, please dont provide `http_conn` to Router.

### 2022.1.3
1. use `lambda` to simplify code.<br/></br>
2. now it can add your own route,I try to imitate the conception like Flask.<br/></br>
3. and config your server param in `./conf/server-config.xml`,<br/>
so you can configure your particular option, and easily pass your conf among computer.



optimize code structure...


# Setup
1. change workspace<br/></br>
`# /usr/bin/bash
cd <"your project path">`<br/></br>

2. compile file<br/></br>
`./build.sh`<br/></br>

3. run the server<br/></br>
`./server`