FROM python:3.12-slim
WORKDIR /app
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY server.py bootstrap.py issue_offline.py admin.html admin.js ./
ENV TS_AUTH_STATE_DIR=/state
VOLUME ["/state"]
EXPOSE 8080
CMD ["uvicorn", "server:app", "--host", "0.0.0.0", "--port", "8080", "--proxy-headers", "--forwarded-allow-ips=127.0.0.1"]
