--
-- PostgreSQL database dump
--

-- Dumped from database version 16.8
-- Dumped by pg_dump version 16.8

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: movies; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.movies (
    id integer NOT NULL,
    title text NOT NULL,
    year integer NOT NULL,
    filepath text
);


ALTER TABLE public.movies OWNER TO postgres;

--
-- Name: users; Type: TABLE; Schema: public; Owner: elik565
--

CREATE TABLE public.users (
    id integer NOT NULL,
    username text NOT NULL,
    password text NOT NULL,
    can_modify boolean DEFAULT false NOT NULL
);


ALTER TABLE public.users OWNER TO elik565;

--
-- Name: users_id_seq; Type: SEQUENCE; Schema: public; Owner: elik565
--

CREATE SEQUENCE public.users_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.users_id_seq OWNER TO elik565;

--
-- Name: users_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: elik565
--

ALTER SEQUENCE public.users_id_seq OWNED BY public.users.id;


--
-- Name: users id; Type: DEFAULT; Schema: public; Owner: elik565
--

ALTER TABLE ONLY public.users ALTER COLUMN id SET DEFAULT nextval('public.users_id_seq'::regclass);


--
-- Data for Name: movies; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.movies (id, title, year, filepath) FROM stdin;
0	Knives Out	2019	
1	Taxi 2	2000	
2	Москва слезам не верит	1979	москва_слезам_не_верит/москва_слезам_не_верит_.m3u8
3	Операция Ы	1965	операция_ы/операция_ы_.m3u8
4	Домовенок Кузя	1986	домовенок_кузя/домовенок_кузя_.m3u8
\.


--
-- Data for Name: users; Type: TABLE DATA; Schema: public; Owner: elik565
--

COPY public.users (id, username, password, can_modify) FROM stdin;
1	admin1	qwerty	t
2	client1	12345	f
5	client2	12345678	f
6	client3	ytrewq	f
\.


--
-- Name: users_id_seq; Type: SEQUENCE SET; Schema: public; Owner: elik565
--

SELECT pg_catalog.setval('public.users_id_seq', 10, true);


--
-- Name: movies movies_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.movies
    ADD CONSTRAINT movies_pkey PRIMARY KEY (id);


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: public; Owner: elik565
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);


--
-- Name: users users_username_key; Type: CONSTRAINT; Schema: public; Owner: elik565
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_username_key UNIQUE (username);


--
-- PostgreSQL database dump complete
--

