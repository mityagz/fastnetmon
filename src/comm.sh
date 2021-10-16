kill -9  $(ps ax|grep fastnetmon|grep -v grep|awk '{print $1}')
rm -rf build; mkdir build; cd build
cmake .. -DDISABLE_PF_RING_SUPPORT=ON 
make
./fastnetmon --daemonize --configuration_file /etc/fastnetmon.conf 
./fastnetmon_client
