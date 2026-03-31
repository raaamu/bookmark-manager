# Import required libraries
from flask import Flask, request, jsonify
from flask_sqlalchemy import SQLAlchemy
from flask_cors import CORS  # Enable Cross-Origin Resource Sharing
from flask_jwt_extended import JWTManager, create_access_token, jwt_required, get_jwt_identity
from werkzeug.security import generate_password_hash, check_password_hash  # For password hashing
import os
from dotenv import load_dotenv  # For environment variables
from datetime import datetime

# Load environment variables from .env file
load_dotenv()

# Initialize Flask application
app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

# Database configuration
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///bookmarks.db'  # SQLite database file
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False  # Disable modification tracking
app.config['JWT_SECRET_KEY'] = os.getenv('JWT_SECRET_KEY', 'your-secret-key')  # JWT secret key

# Initialize extensions
db = SQLAlchemy(app)  # Database ORM
jwt = JWTManager(app)  # JWT manager for authentication

# User model definition
class User(db.Model):
    """User model for storing user information"""
    id = db.Column(db.Integer, primary_key=True)  # Unique identifier
    username = db.Column(db.String(80), unique=True, nullable=False)  # Unique username
    password = db.Column(db.String(120), nullable=False)  # Hashed password
    bookmarks = db.relationship('Bookmark', backref='user', lazy=True)  # One-to-many relationship with bookmarks

# Bookmark model definition
class Bookmark(db.Model):
    """Bookmark model for storing bookmark information"""
    id = db.Column(db.Integer, primary_key=True)  # Unique identifier
    title = db.Column(db.String(200), nullable=False)  # Bookmark title
    url = db.Column(db.String(500), nullable=False)  # Bookmark URL
    description = db.Column(db.String(500))  # Optional description
    created_at = db.Column(db.DateTime, default=datetime.utcnow)  # Creation timestamp
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)  # Update timestamp
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)  # Foreign key to user

# Create database tables
with app.app_context():
    db.create_all()

# Registration endpoint
@app.route('/register', methods=['POST'])
def register():
    """Register a new user"""
    data = request.get_json()
    
    # Validate input
    if not data or 'username' not in data or 'password' not in data:
        return jsonify({'error': 'Missing username or password'}), 400
    
    # Check if username exists
    if User.query.filter_by(username=data['username']).first():
        return jsonify({'error': 'Username already exists'}), 400
    
    # Hash password and create new user
    hashed_password = generate_password_hash(data['password'])
    new_user = User(username=data['username'], password=hashed_password)
    
    try:
        db.session.add(new_user)
        db.session.commit()
        return jsonify({'message': 'User created successfully'}), 201
    except Exception as e:
        db.session.rollback()
        return jsonify({'error': 'Error creating user'}), 500

# Login endpoint
@app.route('/login', methods=['POST'])
def login():
    """Authenticate user and return JWT token"""
    data = request.get_json()
    
    # Validate input
    if not data or 'username' not in data or 'password' not in data:
        return jsonify({'error': 'Missing username or password'}), 400
    
    # Find user and verify password
    user = User.query.filter_by(username=data['username']).first()
    
    if user and check_password_hash(user.password, data['password']):
        access_token = create_access_token(identity=str(user.id))
        return jsonify({'token': access_token}), 200
    
    return jsonify({'error': 'Invalid username or password'}), 401

# Protected bookmark endpoints
@app.route('/bookmarks', methods=['GET'])
@jwt_required()
def get_bookmarks():
    """Get all bookmarks for the authenticated user with optional search"""
    current_user_id = get_jwt_identity()
    
    # Check if search query parameter exists
    search_query = request.args.get('q')
    
    if search_query:
        # Search in title, URL, and description (case-insensitive)
        search_pattern = f"%{search_query}%"
        bookmarks = Bookmark.query.filter_by(user_id=current_user_id).filter(
            (Bookmark.title.ilike(search_pattern)) | 
            (Bookmark.url.ilike(search_pattern)) | 
            (Bookmark.description.ilike(search_pattern))
        ).all()
    else:
        # Get all bookmarks
        bookmarks = Bookmark.query.filter_by(user_id=current_user_id).all()
    
    return jsonify([{
        'id': b.id,
        'title': b.title,
        'url': b.url,
        'description': b.description,
        'created_at': b.created_at.isoformat() if b.created_at else None,
        'updated_at': b.updated_at.isoformat() if b.updated_at else None
    } for b in bookmarks])

@app.route('/bookmarks', methods=['POST'])
@jwt_required()
def add_bookmark():
    """Add a new bookmark for the authenticated user"""
    current_user_id = get_jwt_identity()
    data = request.get_json()
    
    if not data or 'title' not in data or 'url' not in data:
        return jsonify({'error': 'Missing required fields'}), 400
    
    # Check if bookmark with same URL already exists for this user
    existing_bookmark = Bookmark.query.filter_by(user_id=current_user_id, url=data['url']).first()
    if existing_bookmark:
        return jsonify({'error': 'Bookmark with this URL already exists'}), 409
    
    new_bookmark = Bookmark(
        title=data['title'],
        url=data['url'],
        description=data.get('description', ''),
        user_id=current_user_id
    )
    
    try:
        db.session.add(new_bookmark)
        db.session.commit()
        return jsonify({
            'id': new_bookmark.id,
            'title': new_bookmark.title,
            'url': new_bookmark.url,
            'description': new_bookmark.description,
            'created_at': new_bookmark.created_at.isoformat(),
            'updated_at': new_bookmark.updated_at.isoformat()
        }), 201
    except Exception as e:
        db.session.rollback()
        return jsonify({'error': 'Error creating bookmark'}), 500

@app.route('/bookmarks/<int:bookmark_id>', methods=['DELETE'])
@jwt_required()
def delete_bookmark(bookmark_id):
    """Delete a bookmark for the authenticated user"""
    current_user_id = get_jwt_identity()
    bookmark = Bookmark.query.filter_by(id=bookmark_id, user_id=current_user_id).first()
    
    if not bookmark:
        return jsonify({'error': 'Bookmark not found'}), 404
    
    try:
        db.session.delete(bookmark)
        db.session.commit()
        return jsonify({'message': 'Bookmark deleted successfully'}), 200
    except Exception as e:
        db.session.rollback()
        return jsonify({'error': 'Error deleting bookmark'}), 500

# Run the application
if __name__ == '__main__':
    with app.app_context():
        db.create_all()  # Create tables if they don't exist
    app.run(debug=True, host='127.0.0.1', port=5000)  # Run in debug mode on localhost:5000 