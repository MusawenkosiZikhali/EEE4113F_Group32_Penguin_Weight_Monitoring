import math
from flask import Flask, jsonify, render_template, request, send_file, send_from_directory, url_for
from pymongo import MongoClient
from bson import Binary, json_util, ObjectId
import json
from datetime import datetime
from flask_caching import Cache
import requests
import schedule
import time
from threading import Thread
import base64
import os
from io import BytesIO
# from PIL import Image

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
    # Helper function to get the most recent measurement
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
                    'weight': latest['weight_kg'],
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
            mean_weight = round((mean_weight), 2)  # kg/week
        else:
            weight_stddev = 0.0
            mean_weight = max(weights)

        # Calculate weight trend (kg/week)
        weight_change_rate = 0.0
        
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
            'average': mean_weight,
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
        penguin_json = json.loads(json_util.dumps(penguin))
        return jsonify({
            'success': True,
            'penguin': penguin_json,
            'redirect_url': url_for('penguin_detail', penguin_id=penguin_id)
        })
    else:
        return jsonify({
            'success': False,
            'message': f'Penguin {penguin_id} not found'
        }), 404

def get_weather_data(location, timestamp):
    """Fetch weather data for a specific location and time"""
    # Map your locations to coordinates (latitude, longitude)
    weather_api_key = "e4f0bddc1fc2079118ed71df7a9fa6d7"

    location_coords = {
        "Boulders Beach": (-34.1975, 18.4500),
        "Stony Point": (-34.3667, 18.9000),
        "Robben Island": (-33.8066, 18.3662),
        "Betty's Bay": (-34.3644, 18.8906),
        "Halifax Island": (-26.5833, 15.0667),
        "Possession Island": (-26.6167, 15.1167)
    }
    
    if location not in location_coords:
        return None
        
    latitude, longitude = location_coords[location]
    
    try:
        
        # Use current weather API (for recent measurements)
        weather_url = f"https://api.openweathermap.org/data/2.5/weather?lat={latitude}&lon={longitude}&units=metric&appid={weather_api_key}"

        response = requests.get(weather_url)
        response.raise_for_status()
        weather_data = response.json()
        
        return {
            'temperature': weather_data['main']['temp'],
        }
    except Exception as e:
        print(f"Weather API error for {location} at {timestamp}: {str(e)}")
        return None


def map_weight_measurements():
    # Get the weights collection
    weights_collection = db['weights_kg']
    
    # Get all penguins to map device names to penguin_ids
    penguins = list(collection.find({}))
    
    # Creates a mapping of device names to penguin_ids
    device_to_penguin = {p['penguin_id']: p['_id'] for p in penguins}
    
    # Find all weight measurements not yet processed
    # add a 'processed' field to track which measurements have been transferred
    unprocessed_weights = list(weights_collection.find({'processed': {'$exists': False}}))
    
    for weight_measurement in unprocessed_weights:
        device = weight_measurement['device']
        
        if device in device_to_penguin:
            penguin_id = device_to_penguin[device]

            # Process the image if it exists
            image_binary = None
            if 'image' in weight_measurement and weight_measurement['image']:
                try:
                    # Clean the base64 string (remove headers if present)
                    base64_str = weight_measurement['image']
                    if ',' in base64_str:
                        base64_str = base64_str.split(',')[1]
                    
                    # Decode and convert to binary
                    image_binary = Binary(base64.b64decode(base64_str))
                except Exception as e:
                    print(f"Error processing image for {device}: {str(e)}")
                    image_binary = None
            
            # Get weather data
            weather_data = get_weather_data(
                weight_measurement['location'],
                weight_measurement['timestamp']
            )

            # Create the measurement document
            new_measurement = {
                'date': weight_measurement['timestamp'],
                'weight_kg': weight_measurement['weight'],
                'location': weight_measurement['location'],
                'image': image_binary  
            }

            if weather_data:
                new_measurement.update({
                    'temperature': weather_data['temperature'],
                })

            update_operations = {
                '$push': {'measurements': new_measurement}
            }
            
            if image_binary:
                update_operations['$set'] = {'latest_image': image_binary}
            
            collection.update_one(
                {'_id': penguin_id},
                update_operations
            )
            
            # Mark this weight measurement as processed
            weights_collection.update_one(
                {'_id': weight_measurement['_id']},
                {'$set': {'processed': True}}
            )
            
            print(f"Added measurement for penguin {device} at {weight_measurement['timestamp']}")
        else:
            print(f"No penguin found for device {device}")

# Endpoint to get the latest image for a penguin
@app.route('/penguin_latest_image/<penguin_id>')
def get_latest_penguin_image(penguin_id):
    try:
        penguin = collection.find_one(
            {'penguin_id': penguin_id},
            {'latest_image': 1}
        )
        
        if penguin and 'latest_image' in penguin and penguin['latest_image']:
            return send_file(
                BytesIO(penguin['latest_image']),
                mimetype='image/jpeg'
            )
        
        # Use send_from_directory for more reliable static file serving
        return send_from_directory(
            os.path.join(app.root_path, 'static', 'images'),
            'african.jpg',
            mimetype='image/jpeg'
        )
        
    except Exception as e:
        app.logger.error(f"Error serving latest image: {str(e)}")
        try:
            # Fallback to default image even if the first attempt failed
            return send_from_directory(
                os.path.join(app.root_path, 'static', 'images'),
                'default.jpg',
                mimetype='image/jpeg'
            )
        except:
            # Ultimate fallback - return a 404 with no image
            return "Image not found", 404
def run_scheduler():
    # Run the mapping every hour 
    schedule.every().minutes.do(map_weight_measurements)
    
    while True:
        schedule.run_pending()
        time.sleep(1)

# Start the scheduler in a separate thread when the app starts
if __name__ == '__main__':
    scheduler_thread = Thread(target=run_scheduler)
    scheduler_thread.daemon = True
    scheduler_thread.start()
    
    app.run(host='0.0.0.0', port=8080)
