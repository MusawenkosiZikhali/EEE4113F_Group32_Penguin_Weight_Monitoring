import math
from flask import Flask, jsonify, render_template, request, url_for
from pymongo import MongoClient
from bson import json_util, ObjectId
import json
from datetime import datetime
from flask_caching import Cache

app = Flask(__name__)

# MongoDB connection
client = MongoClient(
    'mongodb+srv://musawenkosizikhali7:musa123@testweightpenguin.mudjhr2.mongodb.net/',
    maxPoolSize=50,
    connectTimeoutMS=30000,
    socketTimeoutMS=30000
)
db = client['penguin_db']
collection = db['penguins']

cache = Cache(config={'CACHE_TYPE': 'SimpleCache'})
cache.init_app(app)

def get_latest_measurement(penguin):
    """Helper function to get the most recent measurement"""
    if 'measurements' in penguin and penguin['measurements']:
        # Sort measurements by date (newest first)
        sorted_measurements = sorted(
            penguin['measurements'], 
            key=lambda x: datetime.fromisoformat(x['date']), 
            reverse=True
        )
        return sorted_measurements[0]
    return None

@app.route('/')
@cache.cached(timeout=300)
def index():
    penguins = list(collection.find({}, {'_id': 0})) 
    return render_template('index.html', penguins=penguins)

@app.route('/about')
@cache.cached(timeout=300)
def about():
    penguins = list(collection.find({}, {'_id': 0})) 
    return render_template('about.html', penguins=penguins)

@app.route('/data')
@cache.cached(timeout=300)
def data_explorer():
    try:
        # Get all penguins with their latest measurements
        penguins = list(collection.find({}))
        
        # Process each penguin to include latest measurement data
        processed_penguins = []
        for penguin in penguins:
            latest = get_latest_measurement(penguin)
            if latest:
                processed_penguins.append({
                    '_id': str(penguin['_id']),  # Convert ObjectId to string
                    'penguin_id': penguin['penguin_id'],
                    'species': penguin['species'],
                    'weight': latest['weight_kg'],
                    'condition': latest['condition'],
                    'location': latest['location'],
                    'last_measured': latest['date'],
                })
        
        return render_template('data.html', penguins=processed_penguins)  # Pass all data for JS filtering
    
    except Exception as e:
        print(f"Error loading data: {e}")
        return render_template('data.html',penguins=[])


@app.route('/penguin/<penguin_id>')
def penguin_detail(penguin_id):
    try:
        # Validate penguin_id format if needed (example: check if numeric)
        # if not penguin_id.isalnum():  # Adjust validation as per your ID format
        #     return render_template('error.html', 
        #                         error_code=400,
        #                         error_message="Invalid penguin ID format"), 400

        # Get penguin data with projection to exclude unnecessary fields
        penguin = collection.find_one(
            {'penguin_id': penguin_id},
            {'_id': 0, 'measurements._id': 0}  # Exclude MongoDB _id fields
        )

        if not penguin:
            return render_template('error.html',
                                error_code=404,
                                error_message=f"Penguin {penguin_id} not found"), 404

        # Sort measurements by date (newest first)
        sorted_measurements = sorted(
            penguin['measurements'],
            key=lambda x: datetime.fromisoformat(x['date']),
            reverse=True
        )
        # Extract weight data
        weights = [m['weight_kg'] for m in sorted_measurements]
        dates = [datetime.fromisoformat(m['date']) for m in sorted_measurements]
        
        # Calculate basic statistics
        max_weight = max(weights)
        min_weight = min(weights)
        
        # Calculate standard deviation
        if len(weights) > 1:
            mean_weight = sum(weights) / len(weights)
            variance = sum((x - mean_weight) ** 2 for x in weights) / (len(weights) - 1)
            weight_stddev = round(math.sqrt(variance), 2)
        else:
            weight_stddev = 0.0

        # Calculate weight trend (kg/week)
        weight_change_rate = 0.0
        is_critical = False
        
        if len(sorted_measurements) >= 2:
            # Get the two most recent measurements
            if len(sorted_measurements) >=10:
                recent = sorted_measurements[:10]
                weight_diff = ((recent[0]['weight_kg']-recent[9]['weight_kg'])/recent[9]['weight_kg'])*100
                weight_change_rate = round((weight_diff), 2)  # kg/week
            else:
                recent = sorted_measurements[:]
                weight_diff = ((recent[0]['weight_kg']-recent[-1]['weight_kg'])/recent[-1]['weight_kg'])*100
                weight_change_rate = round((weight_diff), 2)  # kg/week


        # Prepare additional stats for the template
        stats = {
            'weight_history': [m['weight_kg'] for m in sorted_measurements],
            'date_labels': [m['date'][:10] for m in sorted_measurements],
            'first_seen': sorted_measurements[-1]['date'][:10],
            'measurement_count': len(sorted_measurements),
            'max_weight': max_weight,
            'min_weight': min_weight,
            'weight_stddev': weight_stddev,
            'weight_change_rate': weight_change_rate,
            'is_critical':is_critical,
            'critical_threshold': 2
        }

        return render_template(
            'penguin_detail.html',
            penguin=penguin,
            measurements=sorted_measurements,
            stats=stats
        )

    except Exception as e:
        app.logger.error(f"Error loading penguin details: {str(e)}")
        return render_template('error.html',
                            error_code=500,
                            error_message="Internal server error"), 500

@app.template_filter('date_format')
def date_format(value):
    if isinstance(value, str):
        dt = datetime.fromisoformat(value)
        return dt.strftime('%Y-%m-%d')
    return value

@app.route('/api/search_penguin', methods=['GET'])
def api_search_penguin():
    penguin_id = request.args.get('id')
    penguin = collection.find_one({'penguin_id': penguin_id}, {'_id': 0})
    
    if penguin:
        return jsonify({
            'success': True,
            'penguin': penguin,
            'redirect_url': url_for('penguin_detail', penguin_id=penguin_id)
        })
    else:
        return jsonify({
            'success': False,
            'message': f'Penguin {penguin_id} not found'
        }), 404


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)