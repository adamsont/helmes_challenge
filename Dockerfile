FROM gcc
COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
RUN make