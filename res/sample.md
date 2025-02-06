```sql
SELECT "user"."id", "user"."name", "user"."surname", "user"."class", "user"."comment", "plannedabsence"."hours", "plannedabsence"."reason" FROM "plannedabsence" INNER JOIN "user" WHERE "plannedabsence"."id"="user"."id" AND "plannedabsence"."week"=week ORDER BY "user"."surname", "user"."name", "user"."class";
SELECT "contract" FROM "class" WHERE id IN (SELECT class FROM user WHERE id=id);
SELECT "timeslot"."starting_time" FROM "delay" INNER JOIN "timeslot" ON "delay"."timeslot"="timeslot"."id" WHERE "delay"."id"=id AND "state"=state;
SELECT * FROM "user" WHERE "id"=id LIMIT 1;
SELECT * FROM "user" WHERE "id"=id LIMIT 1;
SELECT * FROM "timeslot" WHERE "week"=week ORDER BY "starting_time" ASC;
SELECT "timeslot"."week", "timeslot"."id", "timeslot"."starting_time", "timeslot"."room" FROM "registration" INNER JOIN "timeslot" ON "timeslot"."id"="registration"."id" WHERE "registration"."account"=id AND "timeslot"."week"=week;
SELECT * FROM "timeslot" WHERE "week"=week AND "room"=room ORDER BY "starting_time" ASC;
SELECT "id" FROM "timeslot" WHERE "room"=room AND "whose" LIKE "%" "class" "%" AND "week"=week;
UPDATE "availability" SET "whose"=CONCAT(SUBSTRING_INDEX("whose", class, 1), , SUBSTRING_INDEX("whose", class, -1)) WHERE "room"=room AND "whose" LIKE "%" class "%";
SELECT * FROM "user" WHERE "class"=Vie Scolaire OR "class"=Surveillants;
SELECT "room", COUNT("room") AS count, SUM("it_bool") AS it_bool FROM "availability" GROUP BY "room";
SELECT "room", COUNT("room") AS count, SUM("it_bool") AS it_bool FROM "timeslot" where "week"=:week GROUP BY "room";
SELECT SUM("timeslot"."it_bool") AS SUM FROM "timeslot" INNER JOIN "registration" ON "registration"."id"="timeslot"."id" WHERE "week"=:week AND "registration"."account"=:user_id;
SELECT * from "availability" WHERE "whose" IS NOT NULL AND "room"=:room ;
SELECT count("whose") AS SUM FROM "availability" WHERE "room"=:room AND "whose" like "%" :class "%"AND (;
```
