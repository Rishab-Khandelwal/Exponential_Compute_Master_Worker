worker:
	gcc worker.c -lm -o worker

clean:
	rm -rf worker master

master:
	gcc master.c -o master


run_worker:
	./worker -x 2 -n 10

run_select:
	./master --num_workers 5 --wait_mechanism select -x 2 -n 10 --worker_path ./worker

run_epoll:
	./master --num_workers 5 --wait_mechanism epoll -x 2 -n 10 --worker_path ./worker


build:worker master


check:build run_worker run_select run_epoll
