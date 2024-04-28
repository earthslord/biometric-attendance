import serial
import pandas as pd
import matplotlib.pyplot as plt
#import seaborn as sns
#import numpy as np
from sklearn.cluster import KMeans
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.image import MIMEImage
import datetime
today = datetime.date.today()
ser = serial.Serial("COM3", 9600)
lis = []
endTime = datetime.datetime.now() + datetime.timedelta(seconds=20)
while True:
    if datetime.datetime.now() >= endTime:
        break
    x = ser.readline().decode("ascii")
    print(x)
    if x[0:3] == "VER":
        lis.append([int(x[5:]), datetime.datetime.now().strftime("%H:%M:%S"), datetime.datetime.now().strftime("%Y-%m-%d")])

print(lis[0])
df = pd.read_csv("C:/Users/PRUTHVI/OneDrive/Documents/datasets/finger_dummy_data.csv")
for item in lis:
    df.loc[df.index.max() + 1] = item
df['Date'] = pd.to_datetime(df['Date'])
df['Time'] = pd.to_datetime(df['Time'], format='%H:%M:%S').dt.time

unique_dates = df['Date'].unique()

unique_userids = df['UserID'].unique()

data = {
    'UserID': df['UserID'].values,
    'Date': df['Date'].values
}
user_date = pd.DataFrame(data)
abs_super=[]
for date in unique_dates:
    d = df.loc[df['Date'] == date]
    absentees = list(set(unique_userids) - set(d['UserID']))
    abs_super.append(absentees)

absent = {
    'Date': unique_dates,
    'AttendancePercentage': [i if len(i)>0 else "No absentees" for i in abs_super]
}
absent = pd.DataFrame(absent)

counts = df.groupby(['UserID', 'Date']).size()
duplicate_groups = counts[counts > 1]
duplicates = df[df.set_index(['UserID', 'Date']).index.isin(duplicate_groups.index)]

cleaned_dups = duplicates.sort_values(by=['UserID', 'Date', 'Time']).groupby(['UserID', 'Date']).head(1)
#drop all duplicates and concatenate cleaned data to original df
df_cleaned = df.drop_duplicates(subset=['UserID', 'Date'], keep=False)
final_df = pd.concat([df_cleaned, cleaned_dups], ignore_index=True).sort_values(by=['Date', 'Time'])

final_df['Date'] = pd.to_datetime(final_df['Date'])
daily_attendance = final_df.groupby('Date')['UserID'].nunique()
overall_att = (daily_attendance / len(unique_userids)) * 100
#overall_att['absentees'] = pd.Series(abs_super)


plt.figure(figsize=(10, 5))
overall_att.plot(kind='bar', color='dodgerblue')
plt.title('Attendance Percentage by Day')
plt.xlabel('Date')
plt.ylabel('Attendance Percentage')
plt.xticks(rotation=45)
plt.tight_layout()
plt.savefig('att_by_day.png')
plt.show()

cmap = plt.get_cmap('plasma')
no_days = final_df['Date'].nunique()
user_attendance = final_df.groupby('UserID').size()*100/no_days

normalize = plt.Normalize(vmin=user_attendance.min(), vmax=user_attendance.max())
colors = [cmap(normalize(value)) for value in user_attendance][::-1]

plt.figure(figsize=(8, 5))
plt.bar(user_attendance.index, user_attendance.values, color=colors)
plt.title('Daily attendance by user')
plt.xlabel('User')
plt.ylabel('Percentage')
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.savefig('att_by_user.png')
plt.show()

data = {
    'UserID': user_attendance.index,
    'AttendancePercentage': user_attendance.values
}

cluster_df = pd.DataFrame(data)
X = cluster_df[['AttendancePercentage']]
num_clusters = 3

kmeans = KMeans(n_clusters=num_clusters)
kmeans.fit(X)
cluster_labels = kmeans.labels_

# Add cluster labels to DataFrame
cluster_df['Cluster'] = cluster_labels

# Plot the clusters
plt.figure(figsize=(4, 3))
colors = ['b', 'c', 'm']
labels = ['Low','Moderate','High']
for i in range(num_clusters):
    cluster_data = cluster_df[cluster_df['Cluster'] == i]
    plt.scatter(cluster_data['UserID'], cluster_data['AttendancePercentage'], color=colors[i], label=f'Cluster{i+1}')

plt.xlabel('UserID')
plt.ylabel('Attendance Percentage')
plt.title('Clustering Users by Attendance Percentage')
plt.legend()
plt.grid(True)
plt.savefig('cluster.png')
plt.show()

sender_email = 'pruthviacad24@gmail.com'
receiver_email = 'oxlord03@yahoo.com'
password = 'fseq bspb qisl egyl'

msg = MIMEMultipart()
msg['From'] = sender_email
msg['To'] = receiver_email
msg['Subject'] = 'Attendance Data'

text = f"""
Dear admin,
These are this week's absentees:
{absent}
Please find the attached graphs.

Regards,
Attendance_tech
"""
msg.attach(MIMEText(text, 'plain'))

with open('att_by_day.png', 'rb') as fp:
    img1 = MIMEImage(fp.read())
    img1.add_header('Content-Disposition', 'attachment', filename='att_by_day.png')
    msg.attach(img1)

with open('att_by_user.png', 'rb') as fp:
    img2 = MIMEImage(fp.read())
    img2.add_header('Content-Disposition', 'attachment', filename='att_by_user.png')
    msg.attach(img2)

with open('cluster.png', 'rb') as fp:
    img2 = MIMEImage(fp.read())
    img2.add_header('Content-Disposition', 'attachment', filename='cluster.png')
    msg.attach(img2)

# Connect to SMTP server and send the email
with smtplib.SMTP_SSL('smtp.gmail.com', 465) as smtp:
    smtp.login(sender_email, password)
    smtp.send_message(msg)




