import { setInterval, setTimeout, clearTimeout } from 'worker-timers'

// ui related 
export  class WWTimer {
    // Existing members
    private startTime: number | null = null;
    private timeoutId: number | any;
    private duration: number;
    public isRunning: boolean = false;
    // New member to track if the timer is running
    
  
    constructor(duration: number) {
      this.duration = duration;
    }
  
    // Starts the timer
    start(callback: () => void): void {
      if (this.isRunning) { // Check if the timer is already running
        return;
      }
      this.isRunning = true; // Update isRunning state
      this.startTime = Date.now();
      this.timeoutId = setTimeout(() => {
        callback();
        this.reset();
      }, this.duration);
    }
  
    // Resets the timer
    reset(): void {
      if (this.timeoutId !== null) {
        clearTimeout(this.timeoutId);
        this.timeoutId = null;
      }
      this.startTime = null;
      this.isRunning = false; // Update isRunning state
    }
  
    // Sets a new duration for the timer
    setDuration(newDuration: number): void {
      this.duration = newDuration;
      // If the timer is running, restart it with the new duration
      if (this.isRunning) { // Use isRunning to check if the timer is active
        this.reset();
        this.start(() => {
          console.log("Timer finished after duration update");
        });
      }
    }

    get stopped(): boolean {
        return !this.isRunning;
    }
  }