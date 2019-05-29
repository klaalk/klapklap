LOAD DATA LOCAL INFILE '/Users/micheleluigigreco/progetto_malnati/utility/files/users.csv'
 INTO TABLE USERS
 FIELDS TERMINATED BY ','
 (ID,USERNAME,PSW,EMAIL,NAME,SURNAME,IMAGE,REGISTRATION_DATE);

 LOAD DATA LOCAL INFILE '/Users/micheleluigigreco/progetto_malnati/utility/files/file_owners.csv'
 INTO TABLE FILE_OWNERS
 FIELDS TERMINATED BY ','
 (ID,FILENAME,PATH);
 