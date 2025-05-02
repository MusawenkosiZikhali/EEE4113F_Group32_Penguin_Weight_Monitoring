from pymongo import MongoClient
import json

# Connect to MongoDB
client = MongoClient('mongodb://localhost:27017/')
db = client['penguin_db']
collection = db['penguins']

# Sample data (you can replace this with your actual data)
sample_penguins = [
    {
        "penguin_id": "P001",
        "species": "Adelie",
        "sex": "Male",
        "measurements": [
            {
                "date": "2023-01-15",
                "weight_kg": 4.2,
                "season": "Summer",
                "molting": True,
                "location": "Torgersen Island"
            },
            {
                "date": "2023-04-22",
                "weight_kg": 3.8,
                "season": "Autumn",
                "molting": False,
                "location": "Torgersen Island"
            }
        ]
    },
    {
        "penguin_id": "P002",
        "species": "Gentoo",
        "sex": "Female",
        "measurements": [
            {
                "date": "2023-01-10",
                "weight_kg": 5.1,
                "season": "Summer",
                "molting": False,
                "location": "Biscoe Island"
            }
        ]
    }
]

# Insert data
try:
    result = collection.insert_many(sample_penguins)
    print(f"Inserted {len(result.inserted_ids)} documents")
except Exception as e:
    print(f"Error inserting data: {e}")