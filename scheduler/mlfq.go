package main

import (
	"fmt"
	"flag"
	"strconv"
	"strings"
	"os"
	"math/rand"
)

/**
* Rule 1: If Priority(A) > Priority(B), A runs (B doesn’t).
* Rule 2: If Priority(A) = Priority(B), A & B run in round-robin fashion
		using the time slice (quantum length) of the given queue.
* Rule 3: When a job enters the system, it is placed at the highest
		priority (the topmost queue).
* Rule 4: Once a job uses up its time allotment at a given level 
		(regardless of how many times it has given up the CPU), its priority is
		reduced (i.e., it moves down one queue).
* Rule 5: After some time period S, move all the jobs in the system
		to the topmost queue.
*/


type configParams struct {
	seed int // random seed
	numQueues int // Number of queues in MLFQ
	quantum int // length of time slice
	quantumList []int // lenght of time slice per queue level
	numJobs int // Number of jobs in the system
	maxLen int // max run time of a job
	maxio int // max I/O freq of a job
	boost int // how often to boost the priority f all jobs back to high priority
	ioTime int // how long an I/O should last
	stay bool // reset and stay at the same priority level when issuing I/O
	stats bool // Computes average turnaround and response times.
	allotment int // length of allotment
	allotmentList []int // length of time allotment per queue level
	ioBump bool
}

// Parse command line and initialize config struct
func commandInit(conf *configParams) {
	flag.IntVar(&conf.seed, "seed", 100, "random seed")
	flag.IntVar(&conf.numQueues, "numQueues", 3, "Number of queues in MLFQ")
	flag.IntVar(&conf.quantum, "quantum", 10, "length of time slice")
	strQuantl := flag.String("quantumList", "", "lenght of time slice per queue level, comma seperated")
	flag.IntVar(&conf.numJobs, "numJobs", 3, "Number of jobs in the system")
	flag.IntVar(&conf.maxLen, "maxLen", 100, "max run time of a job")
	flag.IntVar(&conf.maxio, "maxio", 10, "max I/O freq of a job")
	flag.IntVar(&conf.boost, "boost", 0, "how often to boost the priority f all jobs back to high priority")
	flag.IntVar(&conf.ioTime, "ioTime", 5, "how long an I/O should last")
	flag.BoolVar(&conf.stay, "S", false, "reset and stay at the same priority level when issuing I/O")
	flag.BoolVar(&conf.stats, "stats", false, "Computes average turnaround and response times")
	flag.IntVar(&conf.allotment, "allotment", 1, "length of allotment (if not using -allotmentList)")
	strAllotl := flag.String("allotmentList", "", "length of time allotment per queue level, specified as x,y,z,... where x is the # of time slices for the highest priority queue")
	flag.BoolVar(&conf.ioBump, "ioBump", false, "if specified, jobs that finished I/O move immediately to front of current queue")
	flag.Parse()
	// if quantumList provided parse it.
	if *strQuantl != "" {
		strQuantList := strings.Split(*strQuantl, ",")
		for _,v := range strQuantList {
			val, err := strconv.Atoi(v)
			if err != nil {
				fmt.Println("Bad values in quantum list")
				os.Exit(1)
			} else {
				conf.quantumList = append(conf.quantumList, val)
			}
		}
	} else {
		// set all levels same time slice
		for i:=0; i < conf.numQueues; i++ {
			conf.quantumList = append(conf.quantumList, conf.quantum)
		}
	}
	if *strAllotl != "" {
		strAllotList := strings.Split(*strAllotl, ",")
		if len(strAllotList) != len(conf.quantumList) {
			fmt.Println("number of allotments specified must match number of quantums")
			os.Exit(1)
		} else {
			for _,v := range strAllotList {
				val, err := strconv.Atoi(v)
				if err != nil {
					fmt.Println("Bad values in Allotment list")
					os.Exit(1)
				} else {
					conf.allotmentList = append(conf.allotmentList, val)
				}
			}
		}
	} else {
		// set all levels same time allotment
		for i:=0; i < conf.numQueues; i++ {
			conf.allotmentList = append(conf.allotmentList, conf.allotment)
		}
	}
}

// Basically mimics proc_stat
type jobStat struct {
	currPri int
	ticksLeft int
	startTime int
	runTime int
	timeLeft int
	ioFreq int
	doingIO bool
	firstRun int
}

type events struct {
	jobInfo jobStat
	msg string
}

func main() {

	conf := configParams{}
	commandInit(&conf)
	// Tracks when IOs and other interupts are complete at a time/tic
	ioDone := make(map[int][]events)
	// Store info about job
	jobs := make([]jobStat, 0)
	fmt.Println(conf)

	// High queue
	hiQueue := conf.numQueues-1
	//seed
	rand.Seed(int64(conf.seed))

	for i:=0; i< conf.numJobs; i++ {
		jobInfo := jobStat{
			currPri: hiQueue,
			ticksLeft: conf.quantumList[hiQueue],
			startTime: 0,
			runTime: rand.Intn(conf.maxLen),
			ioFreq: rand.Intn(conf.maxio),
			doingIO: false,
			firstRun: -1 }
		jobs = append(jobs, jobInfo)
		ioDone[0] = append(ioDone[0], events{jobInfo, "JOB BEGINS"})
	}

	printBasicInfo(&conf, jobs)

	//MLFQ
	queues := make(map[int][]jobStat)

	// Central time
	currTime := 0

	totalJobs := len(jobs)
	finishedJobs := 0
	fmt.Printf("-------------------------------------\n")
	fmt.Printf("Execution trace:\n")
	for finishedJobs < totalJobs {
		// Find highest priority job
		// Run it until either
		// A] the job uses up its time quantum
		// B] the job performns an I/O

		// Check priority boost
		if conf.boost > 0 && currTime !=0 {
			if currTime % conf.boost == 0{
				fmt.Printf("[time %d] BOOST (every %d)", currTime, conf.boost)
				// Since criteria met, move all jobs to high queue
				for i := 0; i < (conf.numQueues-1); i++ {
					for i,j := range queues[i] {
						if jobs[i].doingIO == false {
							queues[hiQueue] = append(queues[hiQueue], j)
						}
					}
					// Lets empty the queue
					queues[i] = queues[i][len(queues[i]):]
				}

				//Change priority to high priority
				// reset number of ticks left for all jobs
				// Add to highest run queue (if no I/O)
				for i:=0; i < conf.numJobs; i++ {
					if jobs[i].timeLeft > 0 {
						jobs[i].currPri = hiQueue
						jobs[i].ticksLeft = conf.allotmentList[hiQueue]
					}
				}
				fmt.Println("BOOST END: queues", queues)
			}
		}

		// Check if I/Os are done
		_, found := ioDone[currTime]
		if found {
			for i,v := range ioDone[currTime] {
				q := jobs[i].currPri
				jobs[i].doingIO = false
				fmt.Printf("[time %d] %v by JOB %d\n", currTime, v.msg, i)
				if conf.ioBump == false || v.msg == "JOB BEGINS" {
					queues[q] = append(queues[q], v.jobInfo)
				} else {
					// prepend
					queues[q] = append([]jobStat{v.jobInfo}, queues[q]...)
				}
			}
		}

		// Now find the highest priority job
		currQueue := findQueue(hiQueue, queues)
		if currQueue == -1 {
			fmt.Printf("[time %d] IDLE\n", currTime)
			currTime++
			continue
		}

		// There is at least one runnable job
	}
}

// Function to print basic jobs environment
func printBasicInfo(conf *configParams, jobs []jobStat) {
	fmt.Println("List of inputs:")
	fmt.Printf("OPT jobs\t %d\n", conf.numJobs)
	fmt.Printf("OPT queues\t %d\n", conf.numQueues)
	for i, v := range conf.quantumList {
		fmt.Printf("OPT\t quantum lenght for queue %d is %d\n", i, v)
	}
	fmt.Printf("OPT boost\t %d\n", conf.boost)
	fmt.Printf("OPT ioTime\t %d\n", conf.ioTime)
	fmt.Printf("OPT stayAfterIO\t %t\n", conf.stay)
	fmt.Printf("-------------------------------------\n")
	fmt.Printf("\t startTime : at what time does the job enter the system\n")
	fmt.Printf("\t runTime   : the total CPU time needed by the job to finish\n")
	fmt.Printf("\t ioFreq    : every ioFreq time units, the job issues an I/O\n")
	fmt.Printf("Job list:\n")
	for i,v := range jobs {
		fmt.Printf("Job: %d, startTime: %d, runTime: %d, ioFreq: %d\n", i, v.startTime, v.runTime, v.ioFreq)
	}
	if conf.stats != true {
		fmt.Printf("Use -stats to get traces/results when finished.\n")
	}
}

// finds the highest nonempty queue, returns -1 if all are empty
func findQueue(hiQueue int, queues map[int][]jobStat) int {
	q := hiQueue
	for q > 0 {
		if len(queues[q]) > 0 {
			return q
		}
	}
	if len(queues[0]) > 0 {
		return 0
	}
	return -1
}