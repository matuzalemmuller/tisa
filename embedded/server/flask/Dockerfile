FROM python:3.7.2-alpine

WORKDIR /app

COPY requirements.txt /app
COPY flask_test.py /app

RUN pip3 install -r requirements.txt

CMD python3 -u flask_server.py