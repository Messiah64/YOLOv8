import cv2
import argparse

from ultralytics import YOLO


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="YOLOv8 Live Detection")
    parser.add_argument(
        "--model-path",
        default="best.pt",  # Path to your YOLO model
        type=str,
        help="Path to the YOLO model file"
    )
    args = parser.parse_args()
    return args


def main():
    args = parse_arguments()
    model_path = args.model_path

    # Load the YOLO model
    model = YOLO(model_path)

    # Capture video from webcam
    cap = cv2.VideoCapture(0)

    # Loop through video frames
    while True:
        ret, frame = cap.read()

        if not ret:
            print("Error: Unable to capture frame from webcam.")
            break

        # Perform object detection on the frame
        results = model(frame, agnostic_nms=True)[0]

        # Count detected objects
        num_objects = len(results)

        # Display frame with object count
        cv2.putText(frame, f"Objects: {num_objects}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX,
                    1, (0, 255, 0), 2)
        cv2.imshow("YOLO Live Detection", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Release resources
    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
