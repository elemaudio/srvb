<script type='ts'>
  import { onMount } from "svelte";
  import { WWTimer } from './stores/WWTimer'
  import { fade } from "svelte/transition";

  let cablesLoaded = $state(false);
  
  const timers = [ 
    new WWTimer(500),
    new WWTimer(1000),
    new WWTimer(2000)
]

let timedOut = $state(false)

  onMount( () => {
    document.addEventListener("CABLES.jsLoaded", function () {
      cablesLoaded = true;
   })
});

</script> 

<svelte:head>
  <script type="text/javascript" src="/src/cables-js/patch.js" async></script>
  <script type="module" src="/src/cables-js/loader.svelte.js" ></script> 
</svelte:head>

<canvas id="glcanvas" width="100vw" height="100vh" ></canvas>

{#if cablesLoaded }
 { timers[2].start( ()=> timedOut = true )  }
 {#if !timedOut}
 <pre class='console-text' out:fade>NEL SRVB</pre>
 {/if}
{:else}
  <pre class='console-text' in:fade>Loading...</pre>
{/if}

<style>
  .console-text {
    position: absolute;
    left: 1rem;
    top: 1rem;
    color: chartreuse;
  }
</style>