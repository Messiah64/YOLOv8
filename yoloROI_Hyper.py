import cv2
import argparse
import numpy as np
from ultralytics import YOLO
import time
import socket
import ujson
import threading

def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="YOLOv8 Live Detection")
    parser.add_argument(
        "--model-path",
        default="best_3.pt",  # Path to your YOLO model
        type=str,
        help="Path to the YOLO model file"
    )
    args = parser.parse_args()
    return args

def process_roi(model, roi, confidence_threshold, results_list):
    results = model(roi, conf=confidence_threshold, agnostic_nms=True)[0]
    results_list.extend(results)

def main():
    args = parse_arguments()
    
    # Create a socket object
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Host and port for the server
    HOST = '0.0.0.0'  # Listen on all available interfaces
    PORT = 12345

    # Bind the socket to the host and port
    server_socket.bind((HOST, PORT))

    # Listen for incoming connections
    server_socket.listen(1)
    print('Server listening on', PORT)
    model_path = args.model_path

    # Load the YOLO model
    model = YOLO(model_path)

    confidence_threshold = 0.6  # You can adjust this value as needed

    # Capture video from webcam
    cap = cv2.VideoCapture(1)

    start_time = time.time()

    # Loop through video frames
    while True:
        ret, frame = cap.read()
        # Accept incoming connection
        client_socket, client_address = server_socket.accept()
        print('Connected by', client_address)

        if not ret:
            print("Error: Unable to capture frame from webcam.")
            break

        # Divide the frame into four quadrants
        height, width, _ = frame.shape
        half_width = width // 2
        half_height = height // 2

        # Define the four regions of interest
        rois = [
            frame[:half_height, :half_width],
            frame[:half_height, half_width:],
            frame[half_height:, :half_width],
            frame[half_height:, half_width:]
        ]

        # Use threading to process ROIs concurrently
        results_list = []
        threads = []

        for roi in rois:
            thread = threading.Thread(target=process_roi, args=(model, roi, confidence_threshold, results_list))
            threads.append(thread)
            thread.start()

        for thread in threads:
            thread.join()

        # Process the combined results as needed
        object_counts = [len(results) for results in results_list]

        # Draw rectangles around each quadrant and label them
        for i, (x, y, color) in enumerate(zip([0, half_width, 0, half_width], [0, 0, half_height, half_height], 
                                              [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 0)])):
            cv2.rectangle(frame, (x, y), (x + half_width, y + half_height), color, 2)
            cv2.putText(frame, chr(65 + i), (x + half_width // 2, y + half_height // 2), cv2.FONT_HERSHEY_SIMPLEX,
                        1, color, 2)

        # Display object count for each quadrant
        for i, count in enumerate(object_counts):
            cv2.putText(frame, f"{chr(65 + i)} Objects: {count}", 
                        (10 if i < 2 else half_width + 10, 30 if i % 2 == 0 else half_height + 30), 
                        cv2.FONT_HERSHEY_SIMPLEX, 1, ((255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 0))[i], 2)

        cv2.imshow("YOLO Live Detection", frame)

        # Send the array to Server
        data = str(object_counts)
        print(data)
        # Convert array to JSON
        json_data = ujson.dumps(data)
        try:
            # Send JSON data to the client
            client_socket.sendall(json_data.encode())
            print('Data sent successfully')
        except Exception as e:
            print('Error sending data:', e)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Release resources
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
