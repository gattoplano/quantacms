<?php
namespace Quanta\Common;

/**
 * Implements hook_complete().
 * Run the Garbage collector.
 *
 * @param Environment $env
 *   The Environment.
 * @param $vars
 *   An array of variables.
 */
function environment_complete($env, $vars) {
  $garbage = new Garbage($env);
  $garbage->collect();
}