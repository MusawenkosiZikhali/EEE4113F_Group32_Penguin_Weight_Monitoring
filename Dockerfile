# Use official Python image
FROM python:3.9-slim

# Set working directory
WORKDIR /app

# Copy requirements first (caching layer)
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy all files
COPY . .

# Set environment variables
ENV FLASK_APP=App.py
ENV MONGO_URI="mongodb+srv://musawenkosizikhali7:musa123@testweightpenguin.mudjhr2.mongodb.net/"

# Expose port (Cloud Run uses 8080)
EXPOSE 8080

# Run Gunicorn (production WSGI server)
CMD ["gunicorn", "--bind", "0.0.0.0:8080", "app:app"]