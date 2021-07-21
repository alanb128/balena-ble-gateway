FROM balenalib/raspberrypi3-python:3.7-build

WORKDIR /usr/src/app

RUN apt-get update && apt-get install -y libglib2.0-dev

RUN pip3 install bluepy paho-mqtt

CMD ["sleep", "infinity"]
