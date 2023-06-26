import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

import { execSync } from 'node:child_process'

const currentCommit = execSync("git rev-parse --short HEAD").toString();
const date = new Date();
const dateString = `${date.getFullYear()}.${date.getMonth()}.${date.getDate()}`;


// https://vitejs.dev/config/
export default defineConfig({
  base: './',
  define: {
    __COMMIT_HASH__: JSON.stringify(currentCommit),
    __BUILD_DATE__: JSON.stringify(dateString),
  },
  plugins: [react()],
})
