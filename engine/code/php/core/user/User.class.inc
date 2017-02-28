<?php
define("USER_ANONYMOUS", "guest");
define("ROLE_ANONYMOUS", "anonymous");
define("ROLE_ADMIN", "admin");
define("ROLE_LOGGED", "logged");
define("USER_PASSWORD_MIN_LENGTH", 8);
define("USER_ACTION_EDIT", "user_edit");
define("USER_ACTION_REGISTER", "user_register");
define("USER_VALIDATION_ERROR", "validation_error");
define("USER_MIN_NAME_LENGTH", 4);

/**
 * This class represents an user in the system.
 */
class User extends Node {
  public $name;
  public $email;
  // Action being performed on the user
  public $context;
  public $roles = array();
  public $exists;
  public $password;

  /**
   * Get the current navigating user.
   */
  static function current($env, $reload = FALSE) {
    if (!isset($_SESSION['user'])) {
      $_SESSION['user'] = serialize(new User($env, USER_ANONYMOUS));
    }
    $user = unserialize($_SESSION['user']);

    if ($reload) {
      $user = new User($env, $user->name);
    }
    $user->env = $env;

    return $user;
  }

  private function checkPassword($password) {
    return ($this->json->password == pwd($password));
  }

  public function isAnonymous() {
    return $this->name == USER_ANONYMOUS;
  }

  public function load() {
    if ( strlen($this->name) > 0 && $this->exists) {
      $this->loadJSON();
      $this->roles = (array) $this->json->roles;
      if (isset($this->json->password)) {
        $this->password = $this->json->password;
      }
      if (isset($this->json->email)) {
        $this->email = $this->json->email;
      }
      if (isset($this->json->data)) {
        $this->data = (array) $this->json->data;
      }
    }

    $this->env->hook('user_load', array('user' => $this));
  }

  /**
   * Check if the user has a role.
   * @param $role
   * @return bool
   */
  public function hasRole($role) {
    $has_role = FALSE;
    foreach ($this->roles as $k => $user_role) {
      if (trim($user_role) == trim($role)) {
        $has_role = TRUE;
        break;
      }
    }
    return $has_role;
  }

  public function getRoles() {
    if (empty($this->roles)) {
      return array(USER_ANONYMOUS);
    }
    else {
      return $this->roles;
    }
  }

  /**
   * Log out the user.
   */
  public function logOut() {
    new Message($this->env, 'You logged out', MESSAGE_CONFIRM, MESSAGE_TYPE_SCREEN);
    new Message($this->env, 'User ' . $this->name . ' logged out', MESSAGE_CONFIRM, MESSAGE_TYPE_LOG);
    unset($_SESSION['user']);
    $response = new StdClass();
    $response->redirect = '/' . $this->env->getRequestedPath();
    $response_json = json_encode($response);
    return $response_json;
  }

  /**
   * Try to login.
   * @param $password
   */
  public function logIn($password) {
    // If user dir doesn't exist.
    if (!($this->exists)) {
      new Message($this->env, 'This is not a valid username. Please try to [LOGIN] again', MESSAGE_WARNING, MESSAGE_TYPE_SCREEN);
      new Message($this->env, 'Someone tried to login with wrong username: ' . $this->name, MESSAGE_WARNING, MESSAGE_TYPE_LOG);
    }
    else {
      if ($this->checkPassword($password)) {
        new Message($this->env, 'Welcome ' . $this->name . '! You logged in', MESSAGE_CONFIRM, MESSAGE_TYPE_SCREEN);
        new Message($this->env, 'User ' . $this->name . ' logged in', MESSAGE_CONFIRM, MESSAGE_TYPE_LOG);
        $this->roles += array('logged' => 'logged');
        unset($this->password);
        $_SESSION['user'] = serialize($this);
      }
      else {
        new Message($this->env, 'Wrong username or password. Please <a class="login-link" href="#">try again</a>', MESSAGE_WARNING, MESSAGE_TYPE_SCREEN);
        new Message($this->env, 'User ' . $this->name . ' tried to login with wrong username or password', MESSAGE_WARNING, MESSAGE_TYPE_LOG);
      }
    }
    $response = new StdClass();
    $response->redirect = '/' . $this->env->getRequestedPath();
    $response_json = json_encode($response);
    return $response_json;
  }

  public function isCurrent() {
    $curr_user = User::current($this->env);
    return ($curr_user->getName() == $this->getName());
  }

  public function save() {
    $this->env->hook('user_save', array('user' => $this));
    $this->loadJSON();
    $this->saveJSON();

    if ($this->isCurrent()) {
      $this->rebuildSession();
    }
    return TRUE;
  }

  /**
   * User registration / signup.
   * @param $form_data
   * @return bool|void
   */
  private function register($form_data) {
    $this->password = pwd($form_data['edit-password']);
    $this->email = $form_data['edit-email'];
    $this->setData('first_name', $form_data['edit-first_name']);
    $this->setData('last_name', $form_data['edit-last_name']);
    if ($valid = $this->validate($form_data)) {
      $valid = $this->save();
      new Message($this->env, 'User ' . $this->name . ' was registered correctly!', MESSAGE_CONFIRM, MESSAGE_TYPE_SCREEN, 'user');
    }
    return $valid;
  }

  // Todo:update() and register() look too much similar.

  private function update($form_data) {
    foreach ($form_data as $k => $value) {
      $field = str_replace('edit-', '', $k);
      switch ($field) {
        case 'name':
        case 'action':
        case 'context':
        case 'password':
        case 'password_rp':
        case 'old_password':
        case 'new_password':
          break;
        case 'email':
          $this->email = $value;
          break;
        default:
          $this->setData($field, $value);
          break;
      }
    }
    if ($valid = $this->validate($form_data)) {
      if (strlen($form_data['edit-password']) > 0) {
        $this->password = pwd($form_data['edit-password']);
      }
      $valid = $this->save();
    }
    if ($valid) {
      new Message($this->env, 'User ' . $this->name . ' was updated correctly!', MESSAGE_CONFIRM, MESSAGE_TYPE_SCREEN, 'user');
    }
    return $valid;
  }

  /**
   * @param Environment $env
   * @param $action
   * @param $form_data
   */
  public static function requestAction($env, $action, $form_data) {
    // This variable is used to check eventual errors in the user save process.
    $valid = FALSE;
    $curr_user = User::current($env);
    $edit_same_user = $curr_user->name == $form_data['edit-name'];
    $response = new StdClass();
    $can_edit =
      ($action == USER_ACTION_REGISTER && $curr_user->checkAccess(NULL, USER_ACTION_REGISTER)) ||
      ($action == USER_ACTION_EDIT && $curr_user->checkAccess(NULL, USER_ACTION_EDIT)) ||
      ($action == USER_ACTION_EDIT && $edit_same_user && $curr_user->checkAccess(NULL, USER_ACTION_EDIT_OWN));
    if (!$can_edit) {
      new Message($env, 'Sorry, you don\'t have the permissions to perform this action.', MESSAGE_WARNING, MESSAGE_TYPE_SCREEN);
    }
    else {
      $user = new User($env, $form_data['edit-name'], array(ROLE_LOGGED => ROLE_LOGGED));
      $user->context = $action;
      if ($action == USER_ACTION_REGISTER) {
        $valid = $user->register($form_data);
      }
      elseif ($action == USER_ACTION_EDIT) {
        $valid = $user->update($form_data);
      }
    }

    if ($valid) {
      $response->redirect = '/' . $env->getRequestedPath();
      $user->rebuildSession();
    } else {
      $response->errors = Message::burnMessages();
    }
    $response_json = json_encode($response);
    return $response_json;
  }

  public function validate($form_data = array()) {
    $valid = FALSE;
    if ($this->exists) {
      $existing_user = new User($this->env, $this->name);
    }
    $check_password = !$this->exists || strlen($form_data['edit-password']) > 0;

    exec('grep -r -i -o --include \*.json "\"email\"\:\"' . $form_data['edit-email'] . '\"" ' . $this->env->dir['users'], $mail_exists);

    // TODO: check also special characters.
    if (strlen($this->name) < USER_MIN_NAME_LENGTH) {
      new Message($this->env, 'Your username should have at least ' . USER_MIN_NAME_LENGTH . ' characters', MESSAGE_WARNING);
    }
    elseif (!is_alphanumeric($this->name)) {
      new Message($this->env, 'Please use only letters and numbers for your username', MESSAGE_WARNING);
    }

    elseif ($this->exists && $this->context == USER_ACTION_REGISTER) {
      new Message($this->env, 'Username ' . $this->name . ' already exists!', MESSAGE_WARNING);
    }

    elseif ($mail_exists && (!$this->exists || ($this->email != $existing_user->email))) {
      new Message($this->env, 'Email ' . $this->email . ' already exists!', MESSAGE_WARNING);
    }
    elseif(!valid_email($this->email)) {
      new Message($this->env, 'Please enter a valid E-mail address!', MESSAGE_WARNING);
    }
    elseif ($check_password && $this->exists && ($this->password != pwd($form_data['edit-old_password']))) {
      new Message($this->env, 'The old password you entered is not valid. Please try again.', MESSAGE_WARNING);
    }
    elseif ($check_password && strlen($form_data['edit-password']) < USER_PASSWORD_MIN_LENGTH) {
      new Message($this->env, 'Password should be at least ' . USER_PASSWORD_MIN_LENGTH . ' characters long.', MESSAGE_WARNING);
    }
    elseif ($check_password && pwd($form_data['edit-password_rp']) != pwd($form_data['edit-password'])) {
      new Message($this->env, 'Password fields are not the same.', MESSAGE_WARNING);
    }
    else {
      $this->env->hook('user_validate', array('user' => $this, 'form_data' => $form_data));

      $valid = !($this->env->getData(USER_VALIDATION_ERROR));
    }
    return $valid;

  }

  /**
   *
   * Check if user can access a node
   * @return User
   */
  public function checkAccess($node = NULL, $action = NULL) {

    // If node doesn't exist, allow no permission to it.
    if (!$node->exists) {
      return FALSE;
    }

    $permissions = $node->getPermissions();

    $action = trim(strtolower($action));
    $permission = (string) strtolower(trim($permissions[$action]));

    // Strange case in which no permission is set.
    if (!isset($permissions[$action])) {
      new Message($this->env, 'Error: the permission ' . $action . ' does not exist for the node ' . $node->getName() . '.', MESSAGE_ERROR);
      return FALSE;
    }

    $has_permissions = array();
    if (isset($action) && isset($permissions[$action]) && $permission == ROLE_ANONYMOUS) {
      return TRUE;
    }
    foreach ($permissions as $perm_name => $perm_role) {
      if ($this->hasRole($perm_role)) {
        $has_permissions[$perm_name] = $perm_name;
      }
    }

    if (isset($action)) {
      return isset($has_permissions[$action]);
    }
    else {
      return $has_permissions;
    }
  }

  // TODO: refactor.
  public function renderEditForm($context) {
    $this->context = $context;
    $user_edit_form = file_get_contents('core/user/tpl/' . $context . '.inc');
    return $user_edit_form;
  }

  // TODO: refactor.
  public function renderLoginForm() {
    $login_form = file_get_contents('core/user/tpl/user_login.inc');
    return $login_form;
  }

    // TODO
  public function renderUploadAvatarForm($context) {

  }

  public function getName() {
    return $this->name;
  }

  public function getTime() {
    return 0;
  }

  public function getDate() {
    return date('Y-m-d', $this->getTime());
  }

  private function rebuildSession() {
    $_SESSION['user'] = serialize($this);
  }

  public function updateJSON($ignore = array()) {
    // TODO: Implement updateJSON() method.
  }
}