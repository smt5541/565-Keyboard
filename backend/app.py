from flask import Flask, jsonify
from flask_parameter_validation import ValidateParameters, Json


def create_app():
    app = Flask(__name__)

    @app.route("/hello")
    def hello():
        return "Hello, world!"

    @app.get("/get")
    def get():
        return jsonify({"response": "Hello, get!"})

    @app.post("/post")
    @ValidateParameters()
    def post(a: int = Json()):
        return jsonify({"response": f"Hello, post ({a})!"})

    return app