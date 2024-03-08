package main

import (
  "bufio"
  "fmt"
  "io/ioutil"
  "os"
  "os/exec"
  "path/filepath"
  "strconv"
  "strings"
  "sync"
  "time"
)

func main() {
  if len(os.Args) < 4 {
    mainProcess()
  } else if os.Args[3] == "child" {
    processChild()
  }
}

func mainProcess() {
  if len(os.Args) != 3 {
    fmt.Println("Usage: <program> <filename> <number of processes>")
    return
  }

  fileName := os.Args[1]
  processCount, err := strconv.Atoi(os.Args[2])
  if err != nil {
    fmt.Fprintf(os.Stderr, "Error: invalid number of processes format: %s\n", err)
    return
  }

  data, err := ioutil.ReadFile(fileName)
  if err != nil {
    fmt.Fprintf(os.Stderr, "Error reading file: %s\n", err)
    return
  }

  numbersStr := strings.Fields(string(data))
  numbersCount := len(numbersStr)
  if numbersCount < 2 {
    fmt.Println("The file must contain at least 2 numbers.")
    return
  }

  if processCount > numbersCount/2 {
    processCount = numbersCount / 2
    fmt.Printf("Warning: The number of processes has been reduced to: %d\n", processCount)
  }

  numbers := make([]float64, numbersCount)
  for i, s := range numbersStr {
    numbers[i], err = strconv.ParseFloat(s, 64)
    if err != nil {
      fmt.Fprintf(os.Stderr, "Error converting string to float: %s\n", err)
      return
    }
  }

  executablePath, err := os.Executable()
  if err != nil {
    fmt.Fprintf(os.Stderr, "Error getting executable path: %s\n", err)
    return
  }

  executablePath, err = filepath.Abs(executablePath)
  if err != nil {
    fmt.Fprintf(os.Stderr, "Error getting absolute path of executable: %s\n", err)
    return
  }

  var wg sync.WaitGroup
  sum := 0.0
  mutex := &sync.Mutex{}

  portionSize := numbersCount / processCount
  for i := 0; i < processCount; i++ {
    wg.Add(1)
    go func(i int) {
      defer wg.Done()

      start := i * portionSize
      end := start + portionSize
      if i == processCount-1 {
        end = numbersCount // Ensure the last process gets any remaining numbers
      }

      // Write the portion of numbers to a temp file
      tempFile, err := ioutil.TempFile("", "numbers")
      if err != nil {
        fmt.Fprintf(os.Stderr, "Error creating temporary file: %s\n", err)
        return
      }
      defer os.Remove(tempFile.Name())

      for _, num := range numbers[start:end] {
        if _, err := tempFile.WriteString(fmt.Sprintf("%f\n", num)); err != nil {
          fmt.Fprintf(os.Stderr, "Error writing to temporary file: %s\n", err)
          return
        }
      }
      if err := tempFile.Close(); err != nil {
        fmt.Fprintf(os.Stderr, "Error closing temporary file: %s\n", err)
        return
      }

      cmd := exec.Command(executablePath, tempFile.Name(), "1", "child")
      output, err := cmd.CombinedOutput()
      if err != nil {
        fmt.Fprintf(os.Stderr, "Error executing child process: %s\n", err)
        return
      }

      var partSum float64
      if _, err := fmt.Sscanf(string(output), "%f", &partSum); err != nil {
        fmt.Fprintf(os.Stderr, "Error parsing output from child process: %s\n", err)
        return
      }

      mutex.Lock()
      sum += partSum
      mutex.Unlock()
    }(i)
  }

  wg.Wait()

  fmt.Printf("Final sum of squares: %f\n", sum)
}

func processChild() {
  fileName := os.Args[1]

  data, err := ioutil.ReadFile(fileName)
  if err != nil {
    fmt.Fprintf(os.Stderr, "Error reading file: %s\n", err)
    return
  }

  scanner := bufio.NewScanner(strings.NewReader(string(data)))
  var sum float64
  for scanner.Scan() {
    num, err := strconv.ParseFloat(scanner.Text(), 64)
    if err != nil {
      fmt.Fprintf(os.Stderr, "Error converting string to float: %s\n", err)
      return
    }
    sum += num * num
  }

  time.Sleep(5 * time.Second) // Add a 5-second delay as required

  fmt.Printf("%f\n", sum)
}