from supabase import create_client
import pandas as pd
import streamlit as st
import plotly.express as px
import time

st.set_page_config(page_title="Dashboard Cloud Hacks 2", 
                   layout="centered",
                   initial_sidebar_state="collapsed")

API_URL = "https://hjhxraozowuswznxacop.supabase.co"
API_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImhqaHhyYW96b3d1c3d6bnhhY29wIiwicm9sZSI6InNlcnZpY2Vfcm9sZSIsImlhdCI6MTcxOTAwMzk5NiwiZXhwIjoyMDM0NTc5OTk2fQ.4CA6hh5Zo51B4OljhXjGGr7w0U7GQ3ZNxdnNRFA_96Y"

supabase = create_client(API_URL, API_KEY)


def fetch_data():
    supabaseList = supabase.table('maintable').select('*').execute().data

    # Prepare the list to hold all rows
    rows = []

    for row in supabaseList:
        row["created_at"] = row["created_at"].split(".")[0]
        row["time"] = row["created_at"].split("T")[1]
        row["date"] = row["created_at"].split("T")[0]
        row["Datetime"] = row["created_at"]
        rows.append(row)

    # Create DataFrame from the list of rows
    df = pd.DataFrame(rows)
    return df


# Initial data load
df = fetch_data()

# Define the 2 by 2 layout
col1, col2 = st.columns(2)

# Function to display quadrant charts
def display_quadrant_charts():
    with col1:
        st.markdown('### Quadrant 1')
        st.empty()  # Clear previous content in column
        fig1 = px.line(df, x="time", y="q1", title='Quadrant 1', markers=True)
        st.plotly_chart(fig1, use_container_width=True)

        st.markdown('### Quadrant 3')
        st.empty()  # Clear previous content in column
        fig3 = px.line(df, x="time", y="q3", title='Quadrant 3', markers=True)
        st.plotly_chart(fig3, use_container_width=True)

    with col2:
        st.markdown('### Quadrant 2')
        st.empty()  # Clear previous content in column
        fig2 = px.line(df, x="time", y="q2", title='Quadrant 2', markers=True)
        st.plotly_chart(fig2, use_container_width=True)

        st.markdown('### Quadrant 4')
        st.empty()  # Clear previous content in column
        fig4 = px.line(df, x="time", y="q4", title='Quadrant 4', markers=True)
        st.plotly_chart(fig4, use_container_width=True)

# Display initial charts
display_quadrant_charts()

# Rerun the app every 5 seconds
while True:
    time.sleep(5)
    st.rerun()

    # Fetch new data
    df = fetch_data()

    # Clear and display updated charts
    display_quadrant_charts()
