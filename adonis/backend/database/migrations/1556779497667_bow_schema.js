'use strict';

/** @type {import('@adonisjs/lucid/src/Schema')} */
const Schema = use('Schema');

class BowSchema extends Schema {
  up() {
    this.create('bows', table => {
      table.increments();
      table.string('description', 45).notNullable();
      table.timestamps();
    });
  }

  down() {
    this.drop('bows');
  }
}

module.exports = BowSchema;
