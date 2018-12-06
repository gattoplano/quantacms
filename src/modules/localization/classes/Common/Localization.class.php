<?php
namespace Quanta\Common;

/**
 * Class Localization
 *
 * This class manages languages negotiation in the system.
 */
class Localization {
  const LANGUAGE_NEUTRAL = "UND";
  const DIR_LANGUAGES = "_languages";
  public static $system_path = self::DIR_LANGUAGES;

  /**
   * Environment's language is always current one.
   *
   * @param Environment $env
   *   The Environment.
   * @return mixed
   */
  public static function getLanguage(Environment $env) {
    if (!empty($_SESSION['language'])) {
      $lang = $_SESSION['language'];
    }
    else {
      // TODO: we have to check that the fallback language is OK
      // without creating a loop in loading the language node...
      $lang = self::getFallbackLanguage($env);
      // No language set. Set the current language as the fallback language.
      $_SESSION['language'] = $lang;
    }
    return $lang;
  }

  /**
   * Get all enabled languages.
   *
   * @param Environment $env
   *   The Environment.
   *
   * @return array
   *   The languages.
   */
  public static function getEnabledLanguages(Environment $env) {
    // TODO: allow also symlinked language. We need to remove all fallback symlinks (old approach).
    $language_list = $env->scanDirectory($env->dir['languages'], array('symlinks' => 'no'));
    return $language_list;
  }
    /**
   * Get the fallback language.
   *
   * @param Environment $env
   *   The Environment.
   *
   * @return string
   *   The fallback language.
   */
  public static function getFallbackLanguage(Environment $env) {
    $vars = array('fallback_language' => NULL);
    // Let modules set a default fallback language.
    //$env->hook('fallback_language', $vars);

    if (empty($vars['fallback_language'])) {
      $language_list = Localization::getEnabledLanguages($env);
      $fallback = array_pop($language_list);
    }
    else {
      $fallback = $vars['fallback_language'];
    }
    return $fallback;
  }

  /**
   * Switch the current language.
   *
   * @param Environment $env
   *   The Environment.
   *
   * @param string $lang
   *   A language code to switch into.
   */
  public static function switchLanguage(Environment $env, $lang) {
    $language = NodeFactory::load($env, $lang);
    if ($language->exists) {
      $_SESSION['language'] = $lang;
      if (isset($_GET['notify'])){
        new Message($env, 'Language switched to ' . $language->getTitle());
      }
    }
    else {
      new Message($env, 'Error - this language is not enabled: ' . $lang);
    }
  }

  /**
   * Returns a translated version of a string.
   *
   * @param string $string
   *   The translatable string.
   *
   * @param array $replace
   *   An array of replacements to perform in the string.
   *
   * @return string
   *   The translated string.
   */
  public static function t($string, array $replace = array()) {
    // TODO: multilanguage strings implementation.
    foreach ($replace as $k => $replacement) {
      $string = str_replace($k, $replacement, $string);
    }
    return $string;
  }
}
