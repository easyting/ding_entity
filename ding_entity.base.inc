<?php
/**
 * Base class for Ding entity types.
 *
 * Implements getters for properties.
 *
 * Subclasses should define properties with the value DingEntityBase::NULL, to
 * get them automatically converted. When an attemting to access
 * $object->my_prop, DingEntityBase will call $this->getMy_prop, and store the
 * result for future accesses.
 */
class DingEntityBase {
  /**
   * Stores property values.
   */
  protected $properties = array();

  /**
   * Magic value to initialise properties with, to trigger automatic getter
   * method handling.
   */
  const NULL = 'DingEntityBase::undefined';

  /**
   * Class constructor.
   */
  public function __construct() {
    foreach (get_object_vars($this) as $prop => $val) {
      if ($val === self::NULL) {
        // Unset the property, so __get() will take over.
        unset($this->$prop);
        // Set the value to our special null value.
        $this->properties[$prop] = self::NULL;
      }
    }
  }

  /**
   * Wakeup handler.
   */
  public function __wakeup() {
    self::__construct();
  }

  /**
   * Get property value.
   *
   * @param string $prop
   *   Property name.
   *
   * @return mixed
   *   Property value.
   */
  public function __get($prop) {
    if (array_key_exists($prop, $this->properties)) {
      if ($this->properties[$prop] === self::NULL) {
        if (($method = 'get' . strtoupper($prop[0]) . substr($prop, 1)) && method_exists($this, $method)) {
          $this->properties[$prop] = call_user_func(array($this, $method));
        }
        else {
          $trace = debug_backtrace();
          trigger_error(check_plain('No getter defined for ' . $prop .
            ' in ' . $trace[0]['file'] .
            ' on line ' . $trace[0]['line']),
            E_USER_NOTICE);
          return NULL;
        }
      }
      return $this->properties[$prop];
    }

    $trace = debug_backtrace();
    trigger_error(check_plain('Undefined property via __get(): ' . $prop .
      ' in ' . $trace[0]['file'] .
      ' on line ' . $trace[0]['line']),
      E_USER_NOTICE);

    return NULL;
  }

  /**
   * Check if property is set.
   *
   * isset() is defined as whether "a variable is set and is not NULL".
   * If the property does not exist then isset() should always be FALSE.
   *
   * @param string $prop
   *   Property name.
   *
   * @return bool
   *   TRUE if property is set.
   */
  public function __isset($prop) {
    // We cannot just call __get($prop), as that will trigger a NOTICE on
    // non-existing properties, which __isset should not.
    if (!isset($this->properties[$prop])) {
      return FALSE;
    }
    // $this->properties[$prop] needs proper initialization which __get() does.
    // Check whether the value of existing properties is in fact NULL.
    return $this->__get($prop) !== NULL;
  }

  /**
   * Populate object properties from an array.
   *
   * Handles NULL values.
   *
   * @param array $properties
   *   array of properties to copy from $values.
   * @param array $values
   *   associative array of values to copy.
   */
  protected function _populate($properties, $values) {
    foreach ($properties as $prop) {
      // Use array_key_exists to support NULL values.
      if (array_key_exists($prop, $values)) {
        $this->properties[$prop] = $values[$prop];
      }
    }
  }
}

/**
 * Abstract superclass for materials.
 */
abstract class DingEntity extends DingEntityBase {
  /**
   * The unique id of the object.
   */
  public $ding_entity_id;

  /**
   * The local id of the object.
   */
  public $id = DingEntityBase::NULL;

  /**
   * Title of the object.
   */
  public $title = DingEntityBase::NULL;

  /**
   * Creator of the object.
   */
  public $creator = DingEntityBase::NULL;

  /**
   * Abstract (description) of the object.
   */
  public $abstract = DingEntityBase::NULL;

  /**
   * Get entity ID.
   *
   * @return int
   *   Entity ID.
   */
  public function getId() {
    return $this->id = $this->ding_entity_id;
  }

  /**
   * Return whether an object is of a given pseudo-class.
   *
   * Modules can add their own pseudo-classes by implementing
   * hook_ding_entity_is(). This function will call all
   * implementations and return the overall result.
   *
   * Modules may return TRUE, FALSE or NULL from the hook. If any modules
   * returned FALSE, the result is FALSE, else the result is TRUE if anyone
   * returned TRUE, FALSE otherwise.
   */
  public function is($class) {
    $result = module_invoke_all('ding_entity_is', $this, $class);
    // Return true if anyone returned true, and nobody returned false.
    if (!in_array(FALSE, $result) && in_array(TRUE, $result)) {
      return TRUE;
    }
    return FALSE;
  }
}

/**
 * Abstract superclass for collections of materials.
 *
 * Collections is primarily used for grouping together objects together in a
 * 'work'. For instance, a book might be available both as a book, as an
 * audiobook and as an online ebook. These are all part of the same collection
 * representing that work.
 */
abstract class DingEntityCollection extends DingEntityBase {
  /**
   * The unique id of the object.
   */
  public $ding_entity_id;

  /**
   * The local id of the object.
   */
  public $id = DingEntityBase::NULL;

  /**
   * Objects of this collection.
   */
  public $entities = DingEntityBase::NULL;

  /**
   * Get entity ID.
   *
   * @return int
   *   Entity ID.
   */
  public function getId() {
    return $this->id = $this->ding_entity_id;
  }
}
