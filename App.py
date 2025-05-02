from flask import Flask, render_template, request
from pymongo import MongoClient
from bson.json_util import dumps
from bson import json_util
from datetime import datetime
from flask import jsonify
import json

app = Flask(__name__)

# MongoDB connection
client = MongoClient('mongodb://localhost:27017/')
db = client['penguin_db']
collection = db['penguins']

from bson import ObjectId

def build_query(filters):
    """Construct MongoDB query from filters"""
    query = {}
    
    # Species filter
    if filters.get('species'):
        query['species'] = {'$in': filters['species']}
    
    # Sex filter
    if filters.get('sex'):
        query['sex'] = {'$in': filters['sex']}
    
    # Measurements subdocument filters
    measurements_query = {}
    
    # Season filter
    if filters.get('seasons'):
        measurements_query['season'] = {'$in': filters['seasons']}
    
    # Molting filter
    if filters.get('molting') is not None:
        measurements_query['molting'] = filters['molting']
    
    # Location filter
    if filters.get('locations'):
        measurements_query['location'] = {'$in': filters['locations']}
    
    # Combine measurements filters
    if measurements_query:
        query['measurements'] = {'$elemMatch': measurements_query}
    
    return query

@app.route('/debug_data')
def debug_data():
    # Get first 5 documents
    docs = list(collection.find().limit(5))
    return jsonify(docs)  # Requires: from flask import jsonify

@app.route('/')
@app.route('/')
def index():
    # Get unique values for filters
    species = collection.distinct("species")
    seasons = collection.distinct("measurements.season")
    locations = collection.distinct("measurements.location")
    
    # Get raw penguin data (limited to 50 records for performance)
    penguin_data = list(collection.aggregate([
        {'$unwind': '$measurements'},
        {'$limit': 50},
        {'$project': {
            'penguin_id': 1,
            'species': 1,
            'sex': 1,
            'weight_kg': '$measurements.weight_kg',
            'season': '$measurements.season',
            'molting': '$measurements.molting',
            'location': '$measurements.location',
            'date': '$measurements.date'
        }}
    ]))
    
    return render_template('index.html', 
                        species=species, 
                        seasons=seasons, 
                        locations=locations,
                        penguin_data=penguin_data)

# @app.route('/get_data', methods=['POST'])
# def get_data():
#     filters = request.json
    
#     # Build MongoDB query based on filters
#     query = {}
#     if filters.get('species'):
#         query['species'] = {'$in': filters['species']}
#     if filters.get('sex'):
#         query['sex'] = {'$in': filters['sex']}
    
#     # Handle measurements subdocument filters
#     measurements_query = {}
#     if filters.get('seasons'):
#         measurements_query['season'] = {'$in': filters['seasons']}
#     if filters.get('molting') is not None:
#         measurements_query['molting'] = filters['molting']
#     if filters.get('locations'):
#         measurements_query['location'] = {'$in': filters['locations']}
    
#     if measurements_query:
#         query['measurements'] = {'$elemMatch': measurements_query}
    
#     # Aggregate data for visualization
#     pipeline = [
#         {'$unwind': '$measurements'},
#         {'$match': query} if query else {'$match': {}},
#         {'$group': {
#             '_id': {
#                 'species': '$species',
#                 'season': '$measurements.season',
#                 'molting': '$measurements.molting'
#             },
#             'avg_weight': {'$avg': '$measurements.weight_kg'},
#             'count': {'$sum': 1}
#         }}
#     ]
    
#     results = list(collection.aggregate(pipeline))
#     return dumps(results)

@app.route('/get_data', methods=['POST'])
def get_data():
    filters = request.json
    print("Received filters:", filters)  # Debugging
    
    try:
        pipeline = [
            {'$unwind': '$measurements'},
            {'$match': build_query(filters)},
            {'$group': {
                '_id': {
                    'species': '$species',
                    'season': '$measurements.season',
                    'molting': '$measurements.molting'
                },
                'avg_weight': {'$avg': '$measurements.weight_kg'},
                'count': {'$sum': 1}
            }}
        ]
        
        print("Generated pipeline:", pipeline)  # Debugging
        results = list(collection.aggregate(pipeline))
        return json_util.dumps(results)
        
    except Exception as e:
        print("Error in get_data:", str(e))  # Debugging
        return jsonify({'error': str(e)}), 500

@app.template_filter('format_date')
def format_date(value):
    if not value:
        return ''
    try:
        if isinstance(value, str):
            # If it's already a string, return as-is or parse if needed
            return value
        elif isinstance(value, datetime):
            return value.strftime('%Y-%m-%d')
        return str(value)
    except Exception as e:
        print(f"Date formatting error: {e}")
        return str(value)
    
if __name__ == '__main__':
    app.run(debug=True)