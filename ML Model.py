import firebase_admin
from firebase_admin import credentials, db
import pandas as pd
import numpy as np
import time
from sklearn.naive_bayes import GaussianNB
from sklearn import metrics
from sklearn import tree
import warnings

warnings.filterwarnings('ignore')

if not firebase_admin._apps:
    # Initialize Firebase app
    cred = credentials.Certificate("/root/project/FarmerFriend.json")
    firebase_admin.initialize_app(cred, {'databaseURL': 'https://farmerfriend-50b62-default-rtdb.firebaseio.com/'})
    ref = db.reference("/")
    data = ref.get()

# PATH of the data set file
PATH = '/root/project/Crop_recommendation.csv'
df = pd.read_csv(PATH)
precision = 3

if data['Appliances']['RunML'] == "1":
    db.reference("/Appliances").update({"RunML": 0})

    # fetching the data from the firebase
    crop_name = data['Appliances']['label']
    crop_name = crop_name[1:-1]
    user_N = int(data['Appliances']['n'])
    user_P = int(data['Appliances']['p'])
    user_K = int(data['Appliances']['k'])

    if crop_name == "NULL":
        # Extract features and target
        features = df[['N', 'P', 'K']]
        target = df['label']

        # Train a Naive Bayes classifier
        NB = GaussianNB()
        NB.fit(features, target)

        # User input
        user_input = [[user_N, user_P, user_K]]

        # Make predictions on the user input
        predicted_probabilities = NB.predict_proba(user_input)[0]
        predicted_classes = NB.classes_

        # Sort the predicted probabilities
        sorted_indices = np.argsort(predicted_probabilities)[::-1]

        index = sorted_indices[0]
        class_label = predicted_classes[index]
        probability = format(predicted_probabilities[index] * 100, f".{precision}f") + "%"
        db.reference("/Appliances").update({"prob1": class_label})
        db.reference("/Appliances").update({"prob1v": probability})

        index = sorted_indices[1]
        class_label = predicted_classes[index]
        probability = format(predicted_probabilities[index] * 100, f".{precision}f") + "%"
        db.reference("/Appliances").update({"prob2": class_label})
        db.reference("/Appliances").update({"prob2v": probability})

        index = sorted_indices[2]
        class_label = predicted_classes[index]
        probability = format(predicted_probabilities[index] * 100, f".{precision}f") + "%"
        db.reference("/Appliances").update({"prob3": class_label})
        db.reference("/Appliances").update({"prob3v": probability})

        # delay
        time.sleep(1)
        db.reference("/Appliances").update({"Done": 1})

    else:
        # Get the rows corresponding to the crop name
        crop_data = df[df['label'] == crop_name][['N', 'P', 'K']]

        # Calculate the mean NPK values for the crop
        mean_N = crop_data['N'].mean()
        mean_P = crop_data['P'].mean()
        mean_K = crop_data['K'].mean()

        # Calculate the difference between user input and mean values
        diff_N = format(mean_N - user_N, f".{precision}f")
        diff_P = format(mean_P - user_P, f".{precision}f")
        diff_K = format(mean_K - user_K, f".{precision}f")

        # sending results to the database
        db.reference("/Appliances").update({"n2": diff_N})
        db.reference("/Appliances").update({"p2": diff_P})
        db.reference("/Appliances").update({"k2": diff_K})

        time.sleep(1)
        db.reference("/Appliances").update({"Done": 1})
else:
    pass
