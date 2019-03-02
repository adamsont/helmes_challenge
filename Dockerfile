FROM gcc
COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
RUN gcc -o bin/app src/main.c -O3