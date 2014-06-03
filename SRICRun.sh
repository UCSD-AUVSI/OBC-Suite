rfkill unblock 1
ifconfig wlan1 down
iwconfig wlan1 essid "daysinn" key "3018636666"
ifconfig wlan1 up
wget ftp://134.230.1.60:21/12/test.txt
echo "Write down whatever appears below this"
cat test.txt
rfkill block 1
