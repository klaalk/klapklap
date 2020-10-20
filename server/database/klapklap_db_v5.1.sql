-- phpMyAdmin SQL Dump
-- version 4.8.5
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Creato il: Ott 20, 2020 alle 13:07
-- Versione del server: 10.1.38-MariaDB
-- Versione PHP: 5.6.40

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `klapklap`
--

-- --------------------------------------------------------

--
-- Struttura della tabella `files`
--

CREATE TABLE `files` (
  `FILENAME` varchar(178) CHARACTER SET utf32 COLLATE utf32_unicode_ci NOT NULL,
  `HASHNAME` varchar(178) CHARACTER SET utf32 COLLATE utf32_unicode_ci NOT NULL,
  `USERNAME` varchar(178) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `CREATION_DATE` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Struttura della tabella `files_owners`
--

CREATE TABLE `files_owners` (
  `USERNAME` varchar(178) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `HASHNAME` varchar(178) CHARACTER SET utf32 COLLATE utf32_unicode_ci NOT NULL,
  `JOIN_DATE` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `LAST_ACCESS` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `ACTIVE` int(11) NOT NULL DEFAULT '1'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Struttura della tabella `users`
--

CREATE TABLE `users` (
  `ID` int(10) NOT NULL,
  `USERNAME` varchar(255) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `PASSWORD` varbinary(255) NOT NULL,
  `EMAIL` varchar(255) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `ALIAS` varchar(255) DEFAULT NULL,
  `NAME` varchar(255) CHARACTER SET utf16 COLLATE utf16_bin NOT NULL,
  `SURNAME` varchar(255) CHARACTER SET utf16 COLLATE utf16_bin NOT NULL,
  `IMAGE` varchar(64) DEFAULT NULL,
  `REGISTRATION_DATE` timestamp(1) NOT NULL DEFAULT CURRENT_TIMESTAMP(1)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dump dei dati per la tabella `users`
--

INSERT INTO `users` (`ID`, `USERNAME`, `PASSWORD`, `EMAIL`, `ALIAS`, `NAME`, `SURNAME`, `IMAGE`, `REGISTRATION_DATE`) VALUES
(21, 'klauscuko', 0x41774b2b796c6534666353652b3763702b576236, 'klaus.cuko@gmail.com', 'klaus', 'Klaus', 'Cuko', 'Coniglio.jpg', '2020-08-11 13:50:51.0'),
(23, 'bot2', 0x41774c2f697862355049586675765a6f75436537, 'bot2@gmail.com', 'bot2', 'Bot', 'Due', 'Pastore.jpg', '2020-09-06 09:46:40.0'),
(24, 'bot3', 0x41774a7647345a70724256504b6d62344b4c6372, 'bot3@gmail.com', 'imbot3', 'Bot', 'Tre', 'Cinghiale.jpg', '2020-09-06 09:47:30.0'),
(26, 'bot4', 0x41774a6f4849467571784a494c57482f4c374173, 'botq@gmail.com', 'bot490', 'Bot', 'Quattro', 'Topo.jpg', '2020-09-20 14:52:49.0'),
(27, 'paola', 0x41774b614b6c6d655566572b335a494467786b3d, 'paola.caso@gmail.com', 'paola', 'Paola', 'Caso', 'Tigre.jpg', '2020-10-03 15:01:52.0'),
(28, 'albo', 0x41774c666c504574364538576253476a66757436, 'albobruno95@gmail.com', 'albo', 'albo', 'bruno', 'Gorilla.jpg', '2020-10-11 16:50:06.0'),
(29, 'bot5', 0x41774a31415a787a746739564d487a694d713078, 'bot5@gmail.com', 'bot5', 'bot', 'cinque', 'Toro.jpg', '2020-10-18 14:39:03.0');

--
-- Indici per le tabelle scaricate
--

--
-- Indici per le tabelle `files`
--
ALTER TABLE `files`
  ADD PRIMARY KEY (`FILENAME`,`USERNAME`) USING BTREE;

--
-- Indici per le tabelle `files_owners`
--
ALTER TABLE `files_owners`
  ADD PRIMARY KEY (`USERNAME`,`HASHNAME`);

--
-- Indici per le tabelle `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`ID`),
  ADD UNIQUE KEY `USERNAME` (`USERNAME`),
  ADD UNIQUE KEY `EMAIL` (`EMAIL`);

--
-- AUTO_INCREMENT per le tabelle scaricate
--

--
-- AUTO_INCREMENT per la tabella `users`
--
ALTER TABLE `users`
  MODIFY `ID` int(10) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=30;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
