# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
import jlog
from flask import Flask, render_template, send_from_directory, request, redirect, url_for, Response
app = Flask(__name__)
from routes import *
from global_def import *



# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    app.run(debug=False, host='0.0.0.0', port=flask_server_port, threaded=True)


# See PyCharm help at https://www.jetbrains.com/help/pycharm/
