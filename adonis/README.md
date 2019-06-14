# BERGULIX BOW







### Install Node.js

[Nodejs site](https://nodejs.org/en/)
https://nodejs.org/en/download/current/


```shell
$ sudo apt-get install nodejs
```



### Setup MySQL

```shell
$ mysql -u root -p

create database bergulix_bow;
Query OK, 1 row affected (0.001 sec)

CREATE USER 'bergulix_api'@'localhost' IDENTIFIED BY 'secret';
Query OK, 0 rows affected (0.001 sec

GRANT ALL PRIVILEGES ON bergulix_bow.* TO 'bergulix_api'@'localhost';
Query OK, 0 rows affected (0.001 sec)
```



### Install AdonisJS

[AdonisJS site](https://adonisjs.com/docs/4.1/installation)

```shell
$ sudo npm i -g @adonisjs/cli
```



### Do migration

```shell
$ cd bergulix_bow/backend
$ adonis migration:refresh

Already at the last batch
migrate: 1503250034279_user.js
migrate: 1503250034280_token.js
Database migrated successfully in 397 ms
```



### Start backend development server

```shell
$ cd bergulix_bow/backend
$ adonis serve --dev
SERVER STARTED 
> Watching files for changes...

info: serving app on http://127.0.0.1:3333

```

